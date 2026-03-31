#ifndef GNURADIO_DB_CONVERT_HPP
#define GNURADIO_DB_CONVERT_HPP

#include <cmath>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::PowerToDb, [T], [ float, double ])
GR_REGISTER_BLOCK(gr::blocks::math::DbToPower, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct PowerToDb : gr::Block<PowerToDb<T>> {
    using Description = Doc<R""(
@brief Converts a linear power value to decibels: `out = 10 * log10(in / ref)`.

For amplitude (voltage/current) values, set `amplitude_mode = true` to apply
`out = 20 * log10(in / ref)` instead. The reference level `ref` defaults to 1.0.
Input values ≤ 0 produce `-inf`; no clamping is applied.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "ref", Doc<"reference level (denominator of the ratio before log)">, Visible>         ref{T{1}};
    Annotated<bool, "amplitude_mode", Doc<"use 20*log10 for amplitude, 10*log10 for power (default)">> amplitude_mode{false};

    GR_MAKE_REFLECTABLE(PowerToDb, in, out, ref, amplitude_mode);

    [[nodiscard]] constexpr T processOne(T input) const noexcept {
        const T factor = static_cast<bool>(amplitude_mode) ? T{20} : T{10};
        return factor * std::log10(input / static_cast<T>(ref));
    }
};

template<typename T>
requires std::floating_point<T>
struct DbToPower : gr::Block<DbToPower<T>> {
    using Description = Doc<R""(
@brief Converts decibels back to a linear power value: `out = ref * 10^(in / 10)`.

Set `amplitude_mode = true` for amplitude conversion: `out = ref * 10^(in / 20)`.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "ref", Doc<"reference level (multiplied into the result)">, Visible>                  ref{T{1}};
    Annotated<bool, "amplitude_mode", Doc<"use 20 divisor for amplitude, 10 for power (default)">> amplitude_mode{false};

    GR_MAKE_REFLECTABLE(DbToPower, in, out, ref, amplitude_mode);

    [[nodiscard]] constexpr T processOne(T input) const noexcept {
        const T divisor = static_cast<bool>(amplitude_mode) ? T{20} : T{10};
        return static_cast<T>(ref) * std::pow(T{10}, input / divisor);
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_DB_CONVERT_HPP
