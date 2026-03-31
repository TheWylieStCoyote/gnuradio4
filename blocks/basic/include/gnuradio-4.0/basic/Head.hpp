#ifndef GNURADIO_HEAD_HPP
#define GNURADIO_HEAD_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::Head, [T], [ float, double, std::complex<float>, std::complex<double>, std::uint8_t, std::int32_t ])

template<typename T>
struct Head : gr::Block<Head<T>> {
    using Description = Doc<R""(
@brief Forward exactly `n_samples` samples then stop the flowgraph.

Passes samples through unchanged until `n_samples` have been forwarded, then calls
`requestStop()`. Useful for limiting the length of a simulation or recording.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "n_samples", Doc<"number of samples to forward before stopping">, Visible> n_samples{1024U};

    GR_MAKE_REFLECTABLE(Head, in, out, n_samples);

    std::size_t _count{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _count = 0UZ;
    }

    [[nodiscard]] T processOne(T x) noexcept {
        ++_count;
        if (_count >= static_cast<std::size_t>(n_samples)) {
            this->requestStop();
        }
        return x;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_HEAD_HPP
