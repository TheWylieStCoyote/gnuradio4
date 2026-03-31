#ifndef GNURADIO_GOERTZEL_HPP
#define GNURADIO_GOERTZEL_HPP

#include <cmath>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Goertzel, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct Goertzel : gr::Block<Goertzel<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Single-frequency DFT magnitude using Goertzel's second-order recursive algorithm.

Accumulates `block_size` input samples, then emits one output — the DFT magnitude at
`target_frequency`. O(N) per block with far lower overhead than a full FFT when only
one frequency bin is needed (DTMF detection, tone detection, FSK demodulation, power
grid harmonic detection).
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T,          "target_frequency", Doc<"target frequency in Hz">,                    Visible> target_frequency{T{1000}};
    Annotated<T,          "sample_rate",      Doc<"sample rate in Hz">,                         Visible> sample_rate{T{8000}};
    Annotated<gr::Size_t, "block_size",       Doc<"number of samples per output magnitude">,    Visible> block_size{128U};

    GR_MAKE_REFLECTABLE(Goertzel, in, out, target_frequency, sample_rate, block_size);

    T _coeff{};
    T _s1{};
    T _s2{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const T omega = T{2} * std::numbers::pi_v<T> * static_cast<T>(target_frequency) / static_cast<T>(sample_rate);
        _coeff        = T{2} * std::cos(omega);
        _s1           = T{};
        _s2           = T{};
        this->input_chunk_size  = block_size;
        this->output_chunk_size = gr::Size_t{1};
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t N = static_cast<std::size_t>(block_size);

        for (std::size_t outIdx = 0; outIdx < outSpan.size(); ++outIdx) {
            const std::size_t base = outIdx * N;
            if (base + N > inSpan.size()) break;

            T s1{};
            T s2{};
            const T c = static_cast<T>(_coeff);
            for (std::size_t i = 0; i < N; ++i) {
                const T s = inSpan[base + i] + c * s1 - s2;
                s2        = s1;
                s1        = s;
            }
            // power at target frequency (unnormalised)
            outSpan[outIdx] = std::sqrt(s1 * s1 + s2 * s2 - c * s1 * s2);
        }
        return work::Status::OK;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_GOERTZEL_HPP
