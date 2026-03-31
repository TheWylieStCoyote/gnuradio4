#ifndef GNURADIO_THRESHOLD_HPP
#define GNURADIO_THRESHOLD_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Threshold, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct Threshold : gr::Block<Threshold<T>> {
    using Description = Doc<R""(
@brief Hard threshold comparator: emits `high_value` when input exceeds `threshold`, otherwise `low_value`.

No hysteresis — for noise-immune switching see `SchmittTrigger`. Suitable for boolean-valued
control paths and simple level detection.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "threshold",  Doc<"crossing level">,          Visible> threshold{T{0}};
    Annotated<T, "high_value", Doc<"output when in > threshold">, Visible> high_value{T{1}};
    Annotated<T, "low_value",  Doc<"output when in ≤ threshold">, Visible> low_value{T{0}};

    GR_MAKE_REFLECTABLE(Threshold, in, out, threshold, low_value, high_value);

    [[nodiscard]] constexpr T processOne(T input) const noexcept {
        return input > static_cast<T>(threshold) ? static_cast<T>(high_value) : static_cast<T>(low_value);
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_THRESHOLD_HPP
