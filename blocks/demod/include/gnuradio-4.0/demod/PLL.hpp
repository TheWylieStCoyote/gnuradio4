#ifndef GNURADIO_PLL_HPP
#define GNURADIO_PLL_HPP

#include <cmath>
#include <complex>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::demod {

GR_REGISTER_BLOCK(gr::blocks::demod::PLL, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct PLL : gr::Block<PLL<T>> {
    using Description = Doc<R""(
@brief Second-order Phase-Locked Loop (PLL) for phase and frequency tracking.

Accepts a complex baseband input and tracks the carrier phase and frequency.
The loop filter coefficients α and β are derived from `loop_bandwidth` and `damping`
using the Gardner formula. Outputs the phase-corrected complex sample.
)"">;

    using complex_type = std::complex<T>;

    PortIn<complex_type>  in;
    PortOut<complex_type> out;

    Annotated<T, "loop_bandwidth", Doc<"normalised loop bandwidth (BnTs)">>    loop_bandwidth{static_cast<T>(0.01)};
    Annotated<T, "damping",        Doc<"loop damping factor (1/sqrt(2) crit)">> damping{static_cast<T>(0.707)};
    Annotated<T, "max_freq_error", Doc<"maximum frequency offset (rad/sample)">> max_freq_error{static_cast<T>(0.1)};

    GR_MAKE_REFLECTABLE(PLL, in, out, loop_bandwidth, damping, max_freq_error);

    T _phase{};
    T _freq{};
    T _alpha{};
    T _beta{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        // Gardner formula for 2nd-order loop coefficients
        const T Bn     = static_cast<T>(loop_bandwidth);
        const T zeta   = static_cast<T>(damping);
        const T denom  = T{1} + T{2} * zeta * Bn + Bn * Bn;
        _alpha = T{4} * zeta * Bn / denom;
        _beta  = T{4} * Bn * Bn / denom;
        _phase = T{};
        _freq  = T{};
    }

    [[nodiscard]] complex_type processOne(complex_type x) noexcept {
        // Rotate input by current phase estimate
        const complex_type nco  = complex_type{std::cos(_phase), -std::sin(_phase)};
        const complex_type corr = x * nco;

        // Phase error detector: imaginary part of decision-directed product
        const T err = corr.imag() * (corr.real() >= T{} ? T{1} : T{-1});

        // Loop filter update
        _freq  = std::clamp(_freq + _beta * err, -static_cast<T>(max_freq_error), static_cast<T>(max_freq_error));
        _phase = std::fmod(_phase + _freq + _alpha * err, T{2} * std::numbers::pi_v<T>);

        return corr;
    }
};

} // namespace gr::blocks::demod

#endif // GNURADIO_PLL_HPP
