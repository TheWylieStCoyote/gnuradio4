#ifndef GNURADIO_CONJUGATE_HPP
#define GNURADIO_CONJUGATE_HPP

#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Conjugate, [T], [ std::complex<float>, std::complex<double> ])

template<typename T>
requires gr::meta::complex_like<T>
struct Conjugate : gr::Block<Conjugate<T>> {
    using Description = Doc<R""(
@brief Computes the complex conjugate of each input sample: `out[n] = conj(in[n])`.

Negates the imaginary part while leaving the real part unchanged.  Useful for
matched filtering, correlation, and forming the product `x * conj(y)`.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    GR_MAKE_REFLECTABLE(Conjugate, in, out);

    [[nodiscard]] constexpr T processOne(T input) const noexcept { return std::conj(input); }
};

} // namespace gr::blocks::math

#endif // GNURADIO_CONJUGATE_HPP
