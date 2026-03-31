#ifndef GNURADIO_DIFFERENTIATOR_HPP
#define GNURADIO_DIFFERENTIATOR_HPP

#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Differentiator, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Differentiator : gr::Block<Differentiator<T>> {
    using Description = Doc<R""(
@brief First-order backward difference: `out[n] = in[n] − in[n−1]`.

Approximates the discrete derivative of the input stream.  The first output sample
is `in[0] − 0` (i.e. the input itself, as the initial previous value is zero).
Useful for frequency demodulation from an unwrapped phase stream, velocity from
position data, and edge detection.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    GR_MAKE_REFLECTABLE(Differentiator, in, out);

    T _prev{};

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        const T diff = input - _prev;
        _prev        = input;
        return diff;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_DIFFERENTIATOR_HPP
