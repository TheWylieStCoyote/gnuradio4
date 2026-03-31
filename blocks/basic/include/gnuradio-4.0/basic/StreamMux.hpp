#ifndef GNURADIO_STREAM_MUX_HPP
#define GNURADIO_STREAM_MUX_HPP

#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::StreamMux, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct StreamMux : gr::Block<StreamMux<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Round-robin interleave N input streams into one output stream.

Consumes `chunk_size` samples from each of `n_inputs` input ports in order and
appends them sequentially to the output. For `n_inputs = 2` and `chunk_size = 3`,
the output pattern is `[a0 a1 a2 b0 b1 b2 a0 a1 ...]`.
)"">;

    std::vector<PortIn<T>> inputs{};
    PortOut<T>             out;

    Annotated<gr::Size_t, "n_inputs",   Doc<"number of input streams">, Visible> n_inputs{2U};
    Annotated<gr::Size_t, "chunk_size", Doc<"samples per input per round">>      chunk_size{1U};

    GR_MAKE_REFLECTABLE(StreamMux, inputs, out, n_inputs, chunk_size);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        inputs.resize(static_cast<std::size_t>(n_inputs));
        const std::size_t cs    = static_cast<std::size_t>(chunk_size);
        const std::size_t nIn   = static_cast<std::size_t>(n_inputs);
        this->input_chunk_size  = static_cast<gr::Size_t>(cs);
        this->output_chunk_size = static_cast<gr::Size_t>(nIn * cs);
    }

    template<gr::InputSpanLike TInSpan, gr::OutputSpanLike TOutSpan>
    [[nodiscard]] gr::work::Status processBulk(std::span<TInSpan>& ins, TOutSpan& outSpan) noexcept {
        const std::size_t cs  = static_cast<std::size_t>(chunk_size);
        std::size_t       dst = 0UZ;
        for (auto& inSpan : ins) {
            for (std::size_t i = 0UZ; i < cs; ++i) {
                outSpan[dst++] = inSpan[i];
            }
        }
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_STREAM_MUX_HPP
