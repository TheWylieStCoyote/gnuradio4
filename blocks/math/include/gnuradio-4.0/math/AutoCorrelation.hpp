#ifndef GNURADIO_AUTO_CORRELATION_HPP
#define GNURADIO_AUTO_CORRELATION_HPP

#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::AutoCorrelation, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct AutoCorrelation : gr::Block<AutoCorrelation<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Estimates the autocorrelation sequence r[k] = Σ x[n]·conj(x[n-k]) for lags k = 0..max_lag.

Consumes exactly `window_size` input samples per call and emits `max_lag + 1` output values
(lags 0 through max_lag). Normalised by `window_size`. Commonly used for OFDM timing
synchronisation via the cyclic-prefix correlator, carrier frequency offset estimation,
and channel impulse response sounding.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "window_size", Doc<"number of samples per correlation block (≥ 1)">, Visible> window_size{64U};
    Annotated<gr::Size_t, "max_lag",     Doc<"maximum lag to compute (< window_size)">,         Visible> max_lag{16U};

    GR_MAKE_REFLECTABLE(AutoCorrelation, in, out, window_size, max_lag);

    std::vector<T> _buf{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t W = static_cast<std::size_t>(window_size);
        _buf.assign(W, T{});
        this->input_chunk_size  = window_size;
        this->output_chunk_size = static_cast<gr::Size_t>(static_cast<std::size_t>(max_lag) + 1UZ);
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t W   = static_cast<std::size_t>(window_size);
        const std::size_t K   = static_cast<std::size_t>(max_lag) + 1UZ;
        const value_type  inv = value_type{1} / static_cast<value_type>(W);

        for (std::size_t outBase = 0; outBase + K <= outSpan.size(); outBase += K) {
            const std::size_t inBase = (outBase / K) * W;
            if (inBase + W > inSpan.size()) break;

            for (std::size_t k = 0; k < K; ++k) {
                T sum{};
                for (std::size_t n = k; n < W; ++n) {
                    sum += inSpan[inBase + n] * conjT(inSpan[inBase + n - k]);
                }
                outSpan[outBase + k] = sum * inv;
            }
        }
        return work::Status::OK;
    }

private:
    [[nodiscard]] static constexpr T conjT(T x) noexcept {
        if constexpr (gr::meta::complex_like<T>) return std::conj(x);
        else return x;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_AUTO_CORRELATION_HPP
