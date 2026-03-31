#ifndef GNURADIO_STREAM_DEMUX_HPP
#define GNURADIO_STREAM_DEMUX_HPP

#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::StreamDemux, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct StreamDemux : gr::Block<StreamDemux<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Round-robin split one input stream into N output streams.

Reads `n_outputs * chunk_size` input samples and distributes them: the first
`chunk_size` samples go to output 0, the next `chunk_size` to output 1, and so on.
)"">;

    PortIn<T>               in;
    std::vector<PortOut<T>> outputs{};

    Annotated<gr::Size_t, "n_outputs",  Doc<"number of output streams">, Visible> n_outputs{2U};
    Annotated<gr::Size_t, "chunk_size", Doc<"samples per output per round">>      chunk_size{1U};

    GR_MAKE_REFLECTABLE(StreamDemux, in, outputs, n_outputs, chunk_size);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        outputs.resize(static_cast<std::size_t>(n_outputs));
        const std::size_t cs    = static_cast<std::size_t>(chunk_size);
        const std::size_t nOut  = static_cast<std::size_t>(n_outputs);
        this->input_chunk_size  = static_cast<gr::Size_t>(nOut * cs);
        this->output_chunk_size = static_cast<gr::Size_t>(cs);
    }

    template<gr::InputSpanLike TInSpan, gr::OutputSpanLike TOutSpan>
    [[nodiscard]] gr::work::Status processBulk(TInSpan& inSpan, std::span<TOutSpan>& outs) noexcept {
        const std::size_t cs  = static_cast<std::size_t>(chunk_size);
        std::size_t       src = 0UZ;
        for (auto& outSpan : outs) {
            for (std::size_t i = 0UZ; i < cs; ++i) {
                outSpan[i] = inSpan[src++];
            }
        }
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_STREAM_DEMUX_HPP
