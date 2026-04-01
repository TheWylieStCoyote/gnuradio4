#ifndef GNURADIO_SCHMITT_TRIGGER_HPP
#define GNURADIO_SCHMITT_TRIGGER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::SchmittTrigger, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct SchmittTrigger : gr::Block<SchmittTrigger<T>> {
    using Description = Doc<R""(
@brief Hysteretic comparator: transitions to `high_value` only when input rises above
`upper_threshold`, and back to `low_value` only when input falls below `lower_threshold`.

Prevents rapid toggling on noisy signals near a single crossing level. The output is
initialised to `low_value` and state is preserved across `processBulk` calls.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "upper_threshold", Doc<"rising edge trigger level">,   Visible> upper_threshold{static_cast<T>(0.5)};
    Annotated<T, "lower_threshold", Doc<"falling edge trigger level">,  Visible> lower_threshold{static_cast<T>(-0.5)};
    Annotated<T, "high_value",      Doc<"output when state is high">,   Visible> high_value{T{1}};
    Annotated<T, "low_value",       Doc<"output when state is low">,    Visible> low_value{T{0}};

    GR_MAKE_REFLECTABLE(SchmittTrigger, in, out, upper_threshold, lower_threshold, high_value, low_value);

    bool _state{false};

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        if (!_state && input > static_cast<T>(upper_threshold)) {
            _state = true;
        } else if (_state && input < static_cast<T>(lower_threshold)) {
            _state = false;
        }
        return _state ? static_cast<T>(high_value) : static_cast<T>(low_value);
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_SCHMITT_TRIGGER_HPP
