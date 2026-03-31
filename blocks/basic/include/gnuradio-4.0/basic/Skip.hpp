#ifndef GNURADIO_SKIP_HPP
#define GNURADIO_SKIP_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::Skip, [T], [ float, double, std::complex<float>, std::complex<double>, std::uint8_t, std::int32_t ])

template<typename T>
struct Skip : gr::Block<Skip<T>> {
    using Description = Doc<R""(
@brief Suppress the first `n_samples` samples (output zeros), then pass the rest unchanged.

Useful for masking transient start-up artefacts or preambles. During the initial `n_samples`
samples the output is T{} (zero); thereafter input is forwarded unchanged.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "n_samples", Doc<"leading samples to suppress (output zeros)">, Visible> n_samples{0U};

    GR_MAKE_REFLECTABLE(Skip, in, out, n_samples);

    std::size_t _skipped{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _skipped = 0UZ;
    }

    [[nodiscard]] T processOne(T x) noexcept {
        if (_skipped < static_cast<std::size_t>(n_samples)) {
            ++_skipped;
            return T{};
        }
        return x;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_SKIP_HPP
