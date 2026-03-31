#ifndef GNURADIO_CYCLIC_PREFIX_REMOVE_HPP
#define GNURADIO_CYCLIC_PREFIX_REMOVE_HPP

#include <algorithm>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::ofdm {

GR_REGISTER_BLOCK(gr::blocks::ofdm::CyclicPrefixRemove, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct CyclicPrefixRemove : gr::Block<CyclicPrefixRemove<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Remove the cyclic prefix from each received OFDM symbol.

Consumes `fft_size + cp_length` input samples and produces `fft_size` output samples
by discarding the first `cp_length` samples (the cyclic prefix).
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "fft_size",  Doc<"OFDM FFT size">, Visible>           fft_size{64U};
    Annotated<gr::Size_t, "cp_length", Doc<"cyclic prefix length in samples">>   cp_length{16U};

    GR_MAKE_REFLECTABLE(CyclicPrefixRemove, in, out, fft_size, cp_length);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N  = static_cast<std::size_t>(fft_size);
        const std::size_t cp = static_cast<std::size_t>(cp_length);
        this->input_chunk_size  = static_cast<gr::Size_t>(N + cp);
        this->output_chunk_size = static_cast<gr::Size_t>(N);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t cp = static_cast<std::size_t>(cp_length);

        // Discard the first cp samples, copy the rest
        std::ranges::copy(inSpan.subspan(cp), outSpan.begin());

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::ofdm

#endif // GNURADIO_CYCLIC_PREFIX_REMOVE_HPP
