#ifndef GNURADIO_FRACTIONAL_DELAY_LINE_HPP
#define GNURADIO_FRACTIONAL_DELAY_LINE_HPP

#include <cmath>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::FractionalDelayLine, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct FractionalDelayLine : gr::Block<FractionalDelayLine<T>> {
    using Description = Doc<R""(
@brief Delays a signal by a non-integer number of samples using a Lagrange FIR interpolator.

The Lagrange FIR of order `n_taps - 1` is computed in `settingsChanged` for the fractional
part of `delay`; the integer part is handled by the history buffer offset. Suitable for
sub-sample timing alignment in clock recovery, multi-channel coherent combining, and
pre-matched-filter alignment. Note: `delay` must be in [0, n_taps).
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<value_type, "delay",  Doc<"fractional delay in samples [0, n_taps)">, Visible> delay{value_type{0}};
    Annotated<gr::Size_t, "n_taps", Doc<"Lagrange FIR order + 1 (≥ 2)">,           Visible> n_taps{8U};

    GR_MAKE_REFLECTABLE(FractionalDelayLine, in, out, delay, n_taps);

    HistoryBuffer<T>   _history{8};
    std::vector<value_type> _taps{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(n_taps);
        _history            = HistoryBuffer<T>(N);
        computeTaps(N, static_cast<value_type>(delay));
    }

    [[nodiscard]] T processOne(T x) noexcept {
        _history.push_front(x);
        T acc{};
        for (std::size_t k = 0; k < _taps.size(); ++k) {
            acc += _taps[k] * _history[k];
        }
        return acc;
    }

private:
    void computeTaps(std::size_t N, value_type d) {
        _taps.resize(N);
        for (std::size_t k = 0; k < N; ++k) {
            value_type h = value_type{1};
            for (std::size_t m = 0; m < N; ++m) {
                if (m != k) {
                    h *= (d - static_cast<value_type>(m)) / (static_cast<value_type>(k) - static_cast<value_type>(m));
                }
            }
            _taps[k] = h;
        }
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_FRACTIONAL_DELAY_LINE_HPP
