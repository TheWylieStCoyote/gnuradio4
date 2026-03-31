#ifndef GNURADIO_MOVING_AVERAGE_HPP
#define GNURADIO_MOVING_AVERAGE_HPP

#include <bit>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::MovingAverage, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct MovingAverage : gr::Block<MovingAverage<T>> {
    using Description = Doc<R""(
@brief Causal moving average filter over a configurable window of samples.

Computes the mean of the last `length` input samples using an O(1)-per-sample
running-sum implementation. The output during the initial `length - 1` samples
is the mean of however many samples have been received so far (warm-up phase).
)"">;

    // Works for both real (float, double) and complex (std::complex<float/double>) T.
    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "length", Doc<"window length in samples (must be ≥ 1)">, Visible> length{16U};

    GR_MAKE_REFLECTABLE(MovingAverage, in, out, length);

    HistoryBuffer<T> _history{16};
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
            _runningSum -= _history[len - 1]; // oldest sample falls out of the window
        }
        _runningSum += input;
        _history.push_front(input);
        ++_filledCount;
        // Multiply by reciprocal rather than dividing — keeps the same type path for
        // both real and complex T (complex * real scalar avoids complex division).
        const std::size_t n   = std::min(_filledCount, len);
        const value_type  inv = value_type{1} / static_cast<value_type>(n);
        return _runningSum * inv;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_MOVING_AVERAGE_HPP
