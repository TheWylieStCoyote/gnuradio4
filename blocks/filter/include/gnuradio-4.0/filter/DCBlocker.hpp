#ifndef GNURADIO_DC_BLOCKER_HPP
#define GNURADIO_DC_BLOCKER_HPP

#include <bit>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::DCBlocker, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct DCBlocker : gr::Block<DCBlocker<T>> {
    using Description = Doc<R""(
@brief Removes the DC component from a signal using a causal moving-average estimator.

Estimates the local mean of the last `length` samples and subtracts it from each
input sample. The estimator uses an O(1)-per-sample running sum, matching the warm-up
behaviour of MovingAverage: during the initial `length - 1` samples the mean is
computed from however many samples have arrived so far.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "length", Doc<"window length in samples used to estimate DC (must be ≥ 1)">, Visible> length{32U};

    GR_MAKE_REFLECTABLE(DCBlocker, in, out, length);

    HistoryBuffer<T> _history{32};
    T                _runningSum{};
    std::size_t      _filledCount{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t cap = std::bit_ceil(std::max(static_cast<std::size_t>(length), std::size_t{1}));
        _history              = HistoryBuffer<T>(cap);
        _runningSum           = T{};
        _filledCount          = 0UZ;
    }

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        const std::size_t len = static_cast<std::size_t>(length);
        if (_filledCount >= len) {
            _runningSum -= _history[len - 1];
        }
        _runningSum += input;
        _history.push_front(input);
        ++_filledCount;
        const std::size_t n   = std::min(_filledCount, len);
        const value_type  inv = value_type{1} / static_cast<value_type>(n);
        return input - _runningSum * inv;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_DC_BLOCKER_HPP
