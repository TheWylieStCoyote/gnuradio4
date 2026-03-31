#ifndef GNURADIO_TAG_DEBUG_SINK_HPP
#define GNURADIO_TAG_DEBUG_SINK_HPP

#include <print>
#include <string>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/meta/formatter.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::TagDebugSink, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct TagDebugSink : gr::Block<TagDebugSink<T>> {
    using Description = Doc<R""(
@brief Log all received tags to stdout and accumulate them for inspection.

Passes data samples through unchanged. For every tag present on the input, it logs
`[sample=N] key=value ...` to stdout and stores the tag in `_receivedTags` for
programmatic inspection (useful in unit tests). Set `verbose = false` to disable
stdout printing while still accumulating.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<bool, "verbose", Doc<"print tags to stdout">> verbose{true};

    GR_MAKE_REFLECTABLE(TagDebugSink, in, out, verbose);

    std::vector<gr::Tag> _receivedTags{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _receivedTags.clear();
    }

    [[nodiscard]] T processOne(T x) noexcept {
        if (this->inputTagsPresent()) {
            const auto& tag = this->mergedInputTag();
            _receivedTags.push_back(gr::Tag{_receivedTags.size(), tag.map});
            if (static_cast<bool>(verbose)) {
                for (const auto& [k, v] : tag.map) {
                    std::print("[sample={}] {}={}\n", _receivedTags.size() - 1UZ, k, v);
                }
            }
        }
        return x;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_TAG_DEBUG_SINK_HPP
