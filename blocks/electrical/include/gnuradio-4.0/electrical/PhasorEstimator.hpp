#ifndef GNURADIO_PHASOR_ESTIMATOR_HPP
#define GNURADIO_PHASOR_ESTIMATOR_HPP

#include <complex>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::electrical {

GR_REGISTER_BLOCK(gr::electrical::PhasorEstimator, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct PhasorEstimator : gr::Block<PhasorEstimator<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Estimate the phasor (complex amplitude) at a known frequency using the Goertzel algorithm.

For each block of `block_size` input samples, computes one complex output sample whose
magnitude is the amplitude and whose argument is the instantaneous phase of the component
at `target_freq` Hz.
)"">;

    using complex_type = std::complex<T>;

    PortIn<T>            in;
    PortOut<complex_type> out;

    Annotated<gr::Size_t, "block_size",   Doc<"samples per phasor estimate">, Visible> block_size{1024U};
    Annotated<T,          "target_freq",  Doc<"frequency of interest">, Unit<"Hz">>    target_freq{50};
    Annotated<T,          "sample_rate",  Doc<"signal sample rate">, Unit<"Hz">>       sample_rate{10000};

    GR_MAKE_REFLECTABLE(PhasorEstimator, in, out, block_size, target_freq, sample_rate);

    T _coeff{};
    T _sinW{};
    T _cosW{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(block_size);
        this->input_chunk_size  = static_cast<gr::Size_t>(N);
        this->output_chunk_size = 1U;

        const T W = T{2} * std::numbers::pi_v<T> * static_cast<T>(target_freq) / static_cast<T>(sample_rate);
        _coeff    = T{2} * std::cos(W);
        _cosW     = std::cos(W);
        _sinW     = std::sin(W);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<complex_type> outSpan) noexcept {
        T s1{};
        T s2{};
        for (const T x : inSpan) {
            const T s0 = x + _coeff * s1 - s2;
            s2         = s1;
            s1         = s0;
        }
        // X[k] = s1 - s2 * exp(-jW) = (s1 - s2*cosW) + j*(s2*sinW)
        outSpan[0] = complex_type{s1 - s2 * _cosW, s2 * _sinW};
        return gr::work::Status::OK;
    }
};

} // namespace gr::electrical

#endif // GNURADIO_PHASOR_ESTIMATOR_HPP
