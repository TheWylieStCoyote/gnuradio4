#ifndef GNURADIO_ADAPTIVE_LMS_FILTER_HPP
#define GNURADIO_ADAPTIVE_LMS_FILTER_HPP

#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::AdaptiveLmsFilter, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct AdaptiveLmsFilter : gr::Block<AdaptiveLmsFilter<T>> {
    using Description = Doc<R""(
@brief Least-Mean-Squares (LMS) adaptive FIR filter.

Per-sample update: `h[k] += step_size · e[n] · conj(x[n-k])` where `e[n] = desired[n] − y[n]`
is the error signal and `y[n] = Σ h[k]·x[n-k]` is the filter output. The leak factor adds
an L2 regularisation term: `h[k] *= (1 − step_size · leak_factor)`. Useful for echo
cancellation, interference cancellation, and channel equalisation.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortIn<T>  desired;
    PortOut<T> out;
    PortOut<T> error;

    Annotated<gr::Size_t,  "n_taps",     Doc<"FIR filter length">,                              Visible> n_taps{16U};
    Annotated<value_type,  "step_size",  Doc<"LMS step size µ (convergence rate)">,             Visible> step_size{static_cast<value_type>(0.01)};
    Annotated<value_type,  "leak_factor",Doc<"leakage coefficient (0 = no leakage)">>                     leak_factor{value_type{0}};

    GR_MAKE_REFLECTABLE(AdaptiveLmsFilter, in, desired, out, error, n_taps, step_size, leak_factor);

    HistoryBuffer<T> _xHistory{16};
    std::vector<T>   _taps{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(n_taps);
        _xHistory = HistoryBuffer<T>(N);
        _taps.assign(N, T{});
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<const T> desiredSpan,
                                            std::span<T> outSpan, std::span<T> errorSpan) noexcept {
        const std::size_t N  = static_cast<std::size_t>(n_taps);
        const value_type  mu = static_cast<value_type>(step_size);
        const value_type  lk = static_cast<value_type>(leak_factor);

        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            _xHistory.push_front(inSpan[i]);

            T y{};
            for (std::size_t k = 0; k < N; ++k) {
                y += _taps[k] * _xHistory[k];
            }

            const T e = desiredSpan[i] - y;
            outSpan[i]   = y;
            errorSpan[i] = e;

            for (std::size_t k = 0; k < N; ++k) {
                _taps[k] = _taps[k] * (value_type{1} - mu * lk) + mu * e * conjT(_xHistory[k]);
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

} // namespace gr::blocks::filter

#endif // GNURADIO_ADAPTIVE_LMS_FILTER_HPP
