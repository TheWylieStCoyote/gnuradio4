#ifndef GNURADIO_STREAM_TAGGER_HPP
#define GNURADIO_STREAM_TAGGER_HPP

#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::StreamTagger, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct StreamTagger : gr::Block<StreamTagger<T>> {
    using Description = Doc<R""(
@brief Inject a tag into the output stream every `interval` samples.

Publishes a tag with a configurable `key` and a value equal to the current sample
index (`uint64_t`) at positions 0, interval, 2·interval, … . Data samples are passed
through unchanged. Useful for framing, packet marking, or triggering downstream blocks.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t,  "interval", Doc<"samples between successive tags">, Visible> interval{1024U};
    Annotated<std::string, "key",      Doc<"tag key name">>                             key{"stream_tag"};

    GR_MAKE_REFLECTABLE(StreamTagger, in, out, interval, key);

    std::size_t _count{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _count = 0UZ;
    }

    [[nodiscard]] T processOne(T x) noexcept {
        const std::size_t iv = static_cast<std::size_t>(interval);
        if (iv > 0UZ && (_count % iv) == 0UZ) {
            gr::property_map tagMap;
            tagMap.emplace(static_cast<const std::string&>(key), static_cast<uint64_t>(_count));
            this->publishTag(std::move(tagMap));
        }
        ++_count;
        return x;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_STREAM_TAGGER_HPP
