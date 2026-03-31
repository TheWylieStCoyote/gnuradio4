#ifndef GNURADIO_QUADRATURE_DEMOD_HPP
#define GNURADIO_QUADRATURE_DEMOD_HPP

#include <complex>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::QuadratureDemod, [T], [ std::complex<float>, std::complex<double> ])

template<typename T>
requires gr::meta::complex_like<T>
struct QuadratureDemod : gr::Block<QuadratureDemod<T>> {
    using Description = Doc<R""(
@brief FM/PM demodulator that recovers instantaneous frequency from a complex baseband signal.

Computes `y[n] = gain * arg(x[n] * conj(x[n-1]))`, which is proportional to the
instantaneous frequency deviation. Set `gain = sample_rate / (2π * max_deviation)`
to recover a normalised frequency signal, or use `gain = 1` for raw phase-difference
output in radians per sample.
)"">;

    using value_type = typename T::value_type;

    PortIn<T>          in;
    PortOut<value_type> out;

    Annotated<value_type, "gain", Doc<"scales the phase-difference output (1 = radians per sample)">, Visible> gain{value_type{1}};

    GR_MAKE_REFLECTABLE(QuadratureDemod, in, out, gain);

    T _prev{};

    [[nodiscard]] constexpr value_type processOne(T input) noexcept {
        const value_type phase_diff = std::arg(input * std::conj(_prev));
        _prev                       = input;
        return gain * phase_diff;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_QUADRATURE_DEMOD_HPP
