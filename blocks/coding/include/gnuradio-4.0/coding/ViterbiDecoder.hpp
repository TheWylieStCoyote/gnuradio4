#ifndef GNURADIO_VITERBI_DECODER_HPP
#define GNURADIO_VITERBI_DECODER_HPP

#include <algorithm>
#include <bit>
#include <cstdint>
#include <limits>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::ViterbiDecoder, , )

/**
 * @brief Hard-decision Viterbi decoder for convolutional codes.
 *
 * State convention: state s (K-1 bits) = the K-1 LSBs of the encoder shift register
 * after encoding the previous bit. The shift register is maintained exactly as in
 * `ConvEncoder`: SR = (SR_old << 1 | b) & mask_K, with the newest bit at position 0.
 *
 * From state s, encoding hypothesis b:
 *   SR_new   = (s << 1 | b) & mask_K
 *   outputs  = parity(SR_new & poly_r) for each r
 *   next_s   = SR_new & mask_Km1 = (s << 1 | b) & mask_Km1
 *
 * Decoded bit at state s = s & 1 (the input bit b that moved the encoder into state s).
 * Survivors store predecessor states (not input bits), enabling clean traceback.
 */
struct ViterbiDecoder : gr::Block<ViterbiDecoder, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Hard-decision Viterbi decoder for convolutional codes.

Decodes rate-1/N convolutional codes. Each input byte is one coded bit (0 or 1).
Processes `traceback_depth` trellis steps per output block. Generator polynomials and
constraint length must match those used in `ConvEncoder`.

Default polynomials are the NASA standard rate-1/2, K=7 code: [0133, 0171] (octal).
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    Annotated<gr::Size_t,            "constraint_length",    Doc<"shift register length K">>      constraint_length{7U};
    Annotated<std::vector<uint32_t>, "generator_polynomials",Doc<"generator polynomials (octal)">>generator_polynomials{std::vector<uint32_t>{0133U, 0171U}};
    Annotated<gr::Size_t,            "traceback_depth",      Doc<"trellis depth per output block">>traceback_depth{35U};

    GR_MAKE_REFLECTABLE(ViterbiDecoder, in, out, constraint_length, generator_polynomials, traceback_depth);

    static constexpr uint32_t kInf = std::numeric_limits<uint16_t>::max();

    std::size_t              _nStates{64UZ};
    std::size_t              _rate{2UZ};
    uint32_t                 _maskK{127U};
    uint32_t                 _maskKm1{63U};
    std::vector<uint32_t>    _pathMetric{};
    std::vector<uint32_t>    _nextMetric{};
    // survivors[step][next_state] = predecessor_state
    std::vector<std::vector<uint32_t>> _survivors{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const uint32_t K = static_cast<uint32_t>(constraint_length);
        _nStates          = 1UZ << (K - 1U);
        _rate             = generator_polynomials.value.empty() ? 1UZ : generator_polynomials.value.size();
        _maskK            = (1U << K) - 1U;
        _maskKm1          = static_cast<uint32_t>(_nStates - 1U);

        const std::size_t D = static_cast<std::size_t>(traceback_depth);
        _pathMetric.assign(_nStates, kInf);
        _pathMetric[0] = 0U;
        _nextMetric.assign(_nStates, kInf);
        _survivors.assign(D, std::vector<uint32_t>(_nStates, 0U));

        this->input_chunk_size  = static_cast<gr::Size_t>(_rate * D);
        this->output_chunk_size = static_cast<gr::Size_t>(D);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const uint8_t> inSpan, std::span<uint8_t> outSpan) noexcept {
        const auto&    polys = generator_polynomials.value;
        const std::size_t D  = static_cast<std::size_t>(traceback_depth);

        // reset path metrics for block-mode decoding
        std::fill(_pathMetric.begin(), _pathMetric.end(), kInf);
        _pathMetric[0] = 0U;

        // forward pass: build trellis
        std::size_t inIdx = 0UZ;
        for (std::size_t step = 0; step < D; ++step) {
            std::fill(_nextMetric.begin(), _nextMetric.end(), kInf);

            for (uint32_t s = 0U; s < static_cast<uint32_t>(_nStates); ++s) {
                if (_pathMetric[s] == kInf) {
                    continue;
                }
                for (uint32_t b = 0U; b <= 1U; ++b) {
                    const uint32_t sr       = ((s << 1U) | b) & _maskK;
                    const uint32_t nextS    = sr & _maskKm1;

                    uint32_t branchMetric = 0U;
                    for (std::size_t r = 0; r < _rate; ++r) {
                        const uint32_t expected = static_cast<uint32_t>(std::popcount(sr & polys[r])) & 1U;
                        const uint32_t received = static_cast<uint32_t>(inSpan[inIdx + r]) & 1U;
                        branchMetric += (expected != received) ? 1U : 0U;
                    }

                    const uint32_t candidate = _pathMetric[s] + branchMetric;
                    if (candidate < _nextMetric[nextS]) {
                        _nextMetric[nextS]       = candidate;
                        _survivors[step][nextS]  = s; // store predecessor
                    }
                }
            }

            std::swap(_pathMetric, _nextMetric);
            inIdx += _rate;
        }

        // find best final state (minimum accumulated Hamming distance)
        const uint32_t bestState = static_cast<uint32_t>(
            std::distance(_pathMetric.begin(),
                          std::ranges::min_element(_pathMetric)));

        // traceback: decoded bit at state s = s & 1
        uint32_t cur = bestState;
        for (std::size_t step = D; step-- > 0U;) {
            outSpan[step] = static_cast<uint8_t>(cur & 1U);
            cur           = _survivors[step][cur];
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_VITERBI_DECODER_HPP
