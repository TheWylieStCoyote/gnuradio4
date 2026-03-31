#ifndef GNURADIO_VECTOR_TO_STREAM_HPP
#define GNURADIO_VECTOR_TO_STREAM_HPP

#include <algorithm>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/DataSet.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::VectorToStream, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct VectorToStream : gr::Block<VectorToStream<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Unpack the first signal of a DataSet<T> into a flat sample stream.

Consumes one DataSet per call and outputs `vlen` samples from its `signal_values[0..vlen)`.
`vlen` must match the DataSet's signal length; excess or missing samples are silently clamped.
Counterpart to `StreamToVector`.
)"">;

    PortIn<DataSet<T>>  in;
    PortOut<T>          out;

    Annotated<gr::Size_t, "vlen", Doc<"vector length: samples per input DataSet">, Visible> vlen{1024U};

    GR_MAKE_REFLECTABLE(VectorToStream, in, out, vlen);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        this->input_chunk_size  = 1U;
        this->output_chunk_size = vlen;
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const DataSet<T>> inSpan, std::span<T> outSpan) noexcept {
        const DataSet<T>& ds      = inSpan[0];
        const std::size_t toCopy  = std::min(ds.signal_values.size(), outSpan.size());
        std::ranges::copy_n(ds.signal_values.cbegin(), static_cast<std::ptrdiff_t>(toCopy), outSpan.begin());
        if (toCopy < outSpan.size()) {
            std::fill(outSpan.begin() + static_cast<std::ptrdiff_t>(toCopy), outSpan.end(), T{});
        }
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_VECTOR_TO_STREAM_HPP
