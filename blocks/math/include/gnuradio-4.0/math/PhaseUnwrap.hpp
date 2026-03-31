#ifndef GNURADIO_PHASE_UNWRAP_HPP
#define GNURADIO_PHASE_UNWRAP_HPP

#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::PhaseUnwrap, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct PhaseUnwrap : gr::Block<PhaseUnwrap<T>> {
    using Description = Doc<R""(
@brief Removes 2π discontinuities from a wrapped phase signal.

Tracks the accumulated phase offset and adds or subtracts multiples of 2π whenever
the step between consecutive samples exceeds π in magnitude.  The output is a
continuously varying phase suitable for differentiation or phase-deviation measurement.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    GR_MAKE_REFLECTABLE(PhaseUnwrap, in, out);

    T _prev{};
    T _offset{};

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        constexpr T kTwoPi = std::numbers::pi_v<T> * T{2};
        T           diff   = input - _prev;
        // wrap diff into (−π, π]
        while (diff >  std::numbers::pi_v<T>) { diff -= kTwoPi; }
        while (diff <= -std::numbers::pi_v<T>) { diff += kTwoPi; }
        _offset += diff - (input - _prev);
        _prev = input;
        return input + _offset;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_PHASE_UNWRAP_HPP
