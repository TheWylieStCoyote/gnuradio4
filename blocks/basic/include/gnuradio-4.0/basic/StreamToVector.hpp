#ifndef GNURADIO_STREAM_TO_VECTOR_HPP
#define GNURADIO_STREAM_TO_VECTOR_HPP

#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/DataSet.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::StreamToVector, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct StreamToVector : gr::Block<StreamToVector<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Group `vlen` contiguous input samples into a single-signal DataSet<T>.

Consumes exactly `vlen` input samples per output DataSet. The DataSet carries one
signal named "samples" with no axis information. Use `VectorToStream` to reverse
the operation.
)"">;

    PortIn<T>           in;
    PortOut<DataSet<T>> out;

    Annotated<gr::Size_t, "vlen", Doc<"vector length: samples per output DataSet">, Visible> vlen{1024U};

    GR_MAKE_REFLECTABLE(StreamToVector, in, out, vlen);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        this->input_chunk_size  = vlen;
        this->output_chunk_size = 1U;
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<DataSet<T>> outSpan) noexcept {
        const std::size_t N = static_cast<std::size_t>(vlen);

        DataSet<T>& ds   = outSpan[0];
        ds               = DataSet<T>{};
        ds.extents       = {static_cast<int32_t>(N)};
        ds.layout        = gr::LayoutRight{};
        ds.signal_names  = {"samples"};
        ds.signal_values.resize(N);
        std::ranges::copy(inSpan, ds.signal_values.begin());

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_STREAM_TO_VECTOR_HPP
