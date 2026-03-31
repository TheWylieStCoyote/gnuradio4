#ifndef GNURADIO_CYCLIC_PREFIX_ADD_HPP
#define GNURADIO_CYCLIC_PREFIX_ADD_HPP

#include <algorithm>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::ofdm {

GR_REGISTER_BLOCK(gr::blocks::ofdm::CyclicPrefixAdd, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct CyclicPrefixAdd : gr::Block<CyclicPrefixAdd<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Prepend a cyclic prefix to each OFDM symbol.

Consumes `fft_size` input samples per symbol and produces `fft_size + cp_length`
output samples by copying the last `cp_length` input samples before the symbol.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "fft_size",  Doc<"OFDM FFT size">, Visible>           fft_size{64U};
    Annotated<gr::Size_t, "cp_length", Doc<"cyclic prefix length in samples">>   cp_length{16U};

    GR_MAKE_REFLECTABLE(CyclicPrefixAdd, in, out, fft_size, cp_length);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t cp = static_cast<std::size_t>(cp_length);
        this->input_chunk_size  = fft_size;
        this->output_chunk_size = static_cast<gr::Size_t>(static_cast<std::size_t>(fft_size) + cp);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t cp = static_cast<std::size_t>(cp_length);

        // Cyclic prefix: copy last cp samples, then the full symbol
        std::ranges::copy(inSpan.last(cp), outSpan.begin());
        std::ranges::copy(inSpan, outSpan.begin() + static_cast<std::ptrdiff_t>(cp));

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::ofdm

#endif // GNURADIO_CYCLIC_PREFIX_ADD_HPP
