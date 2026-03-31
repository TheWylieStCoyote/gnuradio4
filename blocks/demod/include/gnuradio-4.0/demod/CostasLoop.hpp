#ifndef GNURADIO_COSTAS_LOOP_HPP
#define GNURADIO_COSTAS_LOOP_HPP

#include <cmath>
#include <complex>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::demod {

GR_REGISTER_BLOCK(gr::blocks::demod::CostasLoop, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct CostasLoop : gr::Block<CostasLoop<T>> {
    using Description = Doc<R""(
@brief Costas loop carrier recovery for BPSK/QPSK/8PSK modulation.

Uses a 2nd-order PLL loop filter with a modulation-order–specific phase error
detector. `order` selects the PSK order: 2 (BPSK), 4 (QPSK), 8 (8PSK).
Outputs phase-corrected complex samples.
)"">;

    using complex_type = std::complex<T>;

    PortIn<complex_type>  in;
    PortOut<complex_type> out;

    Annotated<T,          "loop_bandwidth", Doc<"normalised loop bandwidth (BnTs)">>    loop_bandwidth{static_cast<T>(0.01)};
    Annotated<T,          "damping",        Doc<"loop damping factor">>                  damping{static_cast<T>(0.707)};
    Annotated<gr::Size_t, "order",          Doc<"PSK order: 2, 4, or 8">>               order{4U};

    GR_MAKE_REFLECTABLE(CostasLoop, in, out, loop_bandwidth, damping, order);

    T _phase{};
    T _freq{};
    T _alpha{};
    T _beta{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const T Bn    = static_cast<T>(loop_bandwidth);
        const T zeta  = static_cast<T>(damping);
        const T denom = T{1} + T{2} * zeta * Bn + Bn * Bn;
        _alpha = T{4} * zeta * Bn / denom;
        _beta  = T{4} * Bn * Bn / denom;
        _phase = T{};
        _freq  = T{};
    }

    [[nodiscard]] complex_type processOne(complex_type x) noexcept {
        const complex_type nco  = complex_type{std::cos(_phase), -std::sin(_phase)};
        const complex_type corr = x * nco;

        // Phase error detector for PSK order
        T err{};
        const T re = corr.real();
        const T im = corr.imag();
        switch (static_cast<std::size_t>(order)) {
        case 2U:
            // BPSK: err = im * sign(re)
            err = im * (re >= T{} ? T{1} : T{-1});
            break;
        case 4U:
            // QPSK: err = re*sign(im) - im*sign(re)  (tangential error)
            err = re * (im >= T{} ? T{1} : T{-1}) - im * (re >= T{} ? T{1} : T{-1});
            break;
        default:
            // 8PSK: arg(corr^8) / 8 approximation
            {
                const T a8 = std::atan2(im, re) * static_cast<T>(static_cast<std::size_t>(order));
                err        = std::sin(a8) / static_cast<T>(static_cast<std::size_t>(order));
            }
            break;
        }

        _freq  = std::clamp(_freq + _beta * err, static_cast<T>(-0.1), static_cast<T>(0.1));
        _phase = std::fmod(_phase + _freq + _alpha * err, T{2} * std::numbers::pi_v<T>);

        return corr;
    }
};

} // namespace gr::blocks::demod

#endif // GNURADIO_COSTAS_LOOP_HPP
