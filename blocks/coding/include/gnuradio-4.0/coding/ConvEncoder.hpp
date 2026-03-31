#ifndef GNURADIO_CONV_ENCODER_HPP
#define GNURADIO_CONV_ENCODER_HPP

#include <bit>
#include <cstdint>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::ConvEncoder, , )

struct ConvEncoder : gr::Block<ConvEncoder, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Rate-1/N convolutional encoder.

Encodes each input bit (one bit per byte, LSB) using a shift-register of length
`constraint_length`. For each input bit, one output byte is produced per polynomial
in `generator_polynomials`. Rate = 1 / generator_polynomials.size().

Default polynomials are the NASA standard rate-1/2, K=7 code: [0133, 0171] (octal).
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    Annotated<gr::Size_t,            "constraint_length",    Doc<"shift register length K">>       constraint_length{7U};
    Annotated<std::vector<uint32_t>, "generator_polynomials",Doc<"generator polynomials (octal)">> generator_polynomials{std::vector<uint32_t>{0133U, 0171U}};

    GR_MAKE_REFLECTABLE(ConvEncoder, in, out, constraint_length, generator_polynomials);

    uint32_t _shiftReg{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _shiftReg = 0U;
        const auto& polys = generator_polynomials.value;
        this->output_chunk_size = static_cast<gr::Size_t>(polys.empty() ? 1UZ : polys.size());
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const uint8_t> inSpan, std::span<uint8_t> outSpan) noexcept {
        const auto& polys = generator_polynomials.value;
        if (polys.empty()) {
            return gr::work::Status::OK;
        }
        const uint32_t mask = static_cast<uint32_t>((1U << static_cast<uint32_t>(constraint_length)) - 1U);

        std::size_t outIdx = 0UZ;
        for (const uint8_t inputBit : inSpan) {
            _shiftReg = ((_shiftReg << 1U) | static_cast<uint32_t>(inputBit & 0x01U)) & mask;
            for (const uint32_t poly : polys) {
                const uint32_t tapped = _shiftReg & poly;
                outSpan[outIdx++] = static_cast<uint8_t>(static_cast<uint32_t>(std::popcount(tapped)) & 1U);
            }
        }
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_CONV_ENCODER_HPP
