#ifndef GNURADIO_AGC_BLOCK_HPP
#define GNURADIO_AGC_BLOCK_HPP

#include <algorithm>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::AgcBlock, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct AgcBlock : gr::Block<AgcBlock<T>> {
    using Description = Doc<R""(
@brief Automatic Gain Control (AGC): adjusts a scalar gain to maintain a target output power.

Uses an exponential moving average to estimate instantaneous power and adjusts a gain
scalar to bring the output power towards `target_power`. The gain tracks changes in
signal level: `attack_rate` controls how quickly gain rises (signal drops), and
`decay_rate` controls how quickly gain falls (signal rises). Both rates are per-sample
multipliers in (0, 1]; set close to 1 for slow response, small for fast response.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<value_type, "target_power", Doc<"desired output RMS² (linear power, default 1.0)">, Visible>  target_power{value_type{1}};
    Annotated<value_type, "attack_rate",  Doc<"per-sample gain increase when signal is below target (0, 1]">, Visible> attack_rate{static_cast<value_type>(0.001)};
    Annotated<value_type, "decay_rate",   Doc<"per-sample gain decrease when signal is above target (0, 1]">, Visible> decay_rate{static_cast<value_type>(0.001)};
    Annotated<value_type, "max_gain",     Doc<"upper bound on gain (prevents runaway in silence)">, Visible>            max_gain{static_cast<value_type>(65536)};
    Annotated<value_type, "min_gain",     Doc<"lower bound on gain (prevents collapse on strong signals)">, Visible>    min_gain{static_cast<value_type>(1e-6)};

    GR_MAKE_REFLECTABLE(AgcBlock, in, out, target_power, attack_rate, decay_rate, max_gain, min_gain);

    value_type _gain{value_type{1}};

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        const T output = input * static_cast<value_type>(_gain);
        // instantaneous output power
        const value_type power = std::real(output * std::conj(output));
        const value_type tp    = static_cast<value_type>(target_power);
        if (power < tp) {
            _gain *= (value_type{1} + static_cast<value_type>(attack_rate));
        } else {
            _gain *= (value_type{1} - static_cast<value_type>(decay_rate));
        }
        _gain = std::clamp(_gain, static_cast<value_type>(min_gain), static_cast<value_type>(max_gain));
        return output;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_AGC_BLOCK_HPP
