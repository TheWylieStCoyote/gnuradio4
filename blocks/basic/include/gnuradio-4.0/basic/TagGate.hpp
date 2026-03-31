#ifndef GNURADIO_TAG_GATE_HPP
#define GNURADIO_TAG_GATE_HPP

#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::TagGate, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct TagGate : gr::Block<TagGate<T>> {
    using Description = Doc<R""(
@brief Pass or zero-fill samples based on a boolean tag key.

Monitors a configurable `key` in the input tag stream. When a tag carrying that key
is received, the gate opens if the value is `true` and closes if `false`. Samples are
passed through unchanged while the gate is open and replaced with `T{}` (zero) while
it is closed. The initial state is set by `initial_open`.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<std::string, "key",          Doc<"tag key that controls the gate">> key{"gate"};
    Annotated<bool,        "initial_open", Doc<"gate state before first tag">>    initial_open{false};

    GR_MAKE_REFLECTABLE(TagGate, in, out, key, initial_open);

    bool _open{false};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _open = static_cast<bool>(initial_open);
    }

    [[nodiscard]] T processOne(T x) noexcept {
        if (this->inputTagsPresent()) {
            const auto& tagMap = this->mergedInputTag().map;
            const auto  it     = tagMap.find(static_cast<const std::string&>(key));
            if (it != tagMap.end()) {
                _open = it->second.template value_or<bool>(false);
            }
        }
        return _open ? x : T{};
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_TAG_GATE_HPP
