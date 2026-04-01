#ifndef GNURADIO_HEADER_PAYLOAD_DEMUX_HPP
#define GNURADIO_HEADER_PAYLOAD_DEMUX_HPP

#include <string>
#include <tuple>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::HeaderPayloadDemux, [T], [ float, double, std::complex<float>, std::complex<double> ])

/**
 * @brief Tag-triggered burst demultiplexer that separates header and payload sections.
 *
 * Monitors the input for a tag carrying `trigger_tag_key`. On detection it enters header
 * mode and routes `header_length` samples to the `header` output; it then switches to
 * payload mode and routes `payload_length` samples to the `payload` output, after which
 * the block returns to the armed (idle) state. Both outputs produce `T{}` while inactive.
 */
template<typename T>
struct HeaderPayloadDemux : gr::Block<HeaderPayloadDemux<T>> {
    using Description = Doc<R""(
@brief Tag-triggered packet demultiplexer with separate header and payload outputs.

On receiving a tag with `trigger_tag_key`, routes `header_length` samples to `header`
then `payload_length` samples to `payload`. Both outputs produce `T{}` while inactive.
)"">;

    PortIn<T>  in;
    PortOut<T> header;
    PortOut<T> payload;

    Annotated<gr::Size_t,  "header_length",   Doc<"samples to route to the header output">,    Visible> header_length{8U};
    Annotated<gr::Size_t,  "payload_length",  Doc<"samples to route to the payload output">,   Visible> payload_length{64U};
    Annotated<std::string, "trigger_tag_key", Doc<"tag key that arms the demux for a burst">>           trigger_tag_key{"trigger"};

    GR_MAKE_REFLECTABLE(HeaderPayloadDemux, in, header, payload, header_length, payload_length, trigger_tag_key);

    enum class State : uint8_t { Idle, Header, Payload };

    State       _state{State::Idle};
    std::size_t _count{0UZ};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _state = State::Idle;
        _count = 0UZ;
    }

    [[nodiscard]] auto processOne(T x) noexcept -> std::tuple<T, T> {
        const std::size_t hLen = static_cast<std::size_t>(header_length);
        const std::size_t pLen = static_cast<std::size_t>(payload_length);
        const std::string& tKey = static_cast<const std::string&>(trigger_tag_key);

        if (_state == State::Idle && this->inputTagsPresent()) {
            if (this->mergedInputTag().map.contains(tKey)) {
                _state = State::Header;
                _count = 0UZ;
            }
        }

        switch (_state) {
        case State::Idle:
            return {T{}, T{}};

        case State::Header:
            if (++_count >= hLen) {
                _state = State::Payload;
                _count = 0UZ;
            }
            return {x, T{}};

        case State::Payload:
            if (++_count >= pLen) {
                _state = State::Idle;
                _count = 0UZ;
            }
            return {T{}, x};
        }
        return {T{}, T{}}; // unreachable
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_HEADER_PAYLOAD_DEMUX_HPP
