#ifndef GNURADIO_MOVING_RMS_HPP
#define GNURADIO_MOVING_RMS_HPP

#include <bit>
#include <cmath>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::MovingRms, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct MovingRms : gr::Block<MovingRms<T>> {
    using Description = Doc<R""(
@brief Causal root-mean-square estimator over a sliding window.

Computes `sqrt(mean(|x|²))` over the last `length` samples using an O(1)-per-sample
running-sum implementation. The output type is always real (`value_type`), even when
the input is complex. The warm-up phase averages only the samples received so far.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>           in;
    PortOut<value_type> out;

    Annotated<gr::Size_t, "length", Doc<"window length in samples (must be ≥ 1)">, Visible> length{16U};

    GR_MAKE_REFLECTABLE(MovingRms, in, out, length);

    HistoryBuffer<value_type> _powerHistory{16};
    value_type                _runningPower{};
    std::size_t               _filledCount{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t cap = std::bit_ceil(std::max(static_cast<std::size_t>(length), std::size_t{1}));
        _powerHistory         = HistoryBuffer<value_type>(cap);
        _runningPower         = value_type{};
        _filledCount          = 0UZ;
    }

    [[nodiscard]] constexpr value_type processOne(T input) noexcept {
        const std::size_t len   = static_cast<std::size_t>(length);
        const value_type  power = std::real(input * std::conj(input)); // |x|²
        if (_filledCount >= len) {
            _runningPower -= _powerHistory[len - 1];
        }
        _runningPower += power;
        _powerHistory.push_front(power);
        ++_filledCount;
        const std::size_t n   = std::min(_filledCount, len);
        const value_type  inv = value_type{1} / static_cast<value_type>(n);
        return std::sqrt(_runningPower * inv);
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_MOVING_RMS_HPP
