#ifndef GNURADIO_INSTANTANEOUS_FREQUENCY_HPP
#define GNURADIO_INSTANTANEOUS_FREQUENCY_HPP

#include <complex>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::InstantaneousFrequency, [T], [ std::complex<float>, std::complex<double> ])

template<typename T>
requires gr::meta::complex_like<T>
struct InstantaneousFrequency : gr::Block<InstantaneousFrequency<T>> {
    using Description = Doc<R""(
@brief Computes the instantaneous frequency of a complex analytic signal.

Per-sample output: `f[n] = arg(x[n] · conj(x[n-1])) · sample_rate / (2π)`.
Set `sample_rate = 1` to obtain normalised frequency in cycles/sample.
Equivalent to `PhaseUnwrap` followed by `Differentiator` but avoids an
intermediate port and the 2π normalisation step.
Output is zero for the first sample (no previous sample available).
)"">;

    using value_type = typename T::value_type;

    PortIn<T>          in;
    PortOut<value_type> out;

    Annotated<value_type, "sample_rate", Doc<"sample rate in Hz; set to 1 for normalised frequency">, Visible> sample_rate{value_type{1}};

    GR_MAKE_REFLECTABLE(InstantaneousFrequency, in, out, sample_rate);

    T _prev{};

    [[nodiscard]] value_type processOne(T input) noexcept {
        const T        product = input * std::conj(_prev);
        const value_type freq  = std::arg(product) * static_cast<value_type>(sample_rate) / (value_type{2} * std::numbers::pi_v<value_type>);
        _prev                  = input;
        return freq;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_INSTANTANEOUS_FREQUENCY_HPP
