#ifndef GNURADIO_IFFT_HPP
#define GNURADIO_IFFT_HPP

#include <bit>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/fourier/fft.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fft {

GR_REGISTER_BLOCK("gr::blocks::fft::IFFT", gr::blocks::fft::IFFT, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct IFFT : gr::Block<IFFT<T>, gr::Resampling<1024LU, 1024LU>> {
    using Description = Doc<R""(
@brief Inverse Fast Fourier Transform (IFFT) — complex in, complex out.

Consumes a block of `fft_size` complex samples representing a frequency-domain frame
and produces `fft_size` time-domain complex samples. The inverse is computed using
the conjugate symmetry identity: IFFT(X) = conj(FFT(conj(X))) / N.
)"">;

    using complex_type = std::complex<T>;

    PortIn<complex_type>  in;
    PortOut<complex_type> out;

    Annotated<gr::Size_t, "fft_size", Doc<"number of complex samples per frame">, Visible> fft_size{1024U};

    GR_MAKE_REFLECTABLE(IFFT, in, out, fft_size);

    gr::algorithm::FFT<complex_type, complex_type> _fft{};
    std::vector<complex_type>                      _work{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N     = static_cast<std::size_t>(fft_size);
        this->input_chunk_size  = static_cast<gr::Size_t>(N);
        this->output_chunk_size = static_cast<gr::Size_t>(N);
        _work.resize(N);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const complex_type> inSpan, std::span<complex_type> outSpan) noexcept {
        const std::size_t N    = static_cast<std::size_t>(fft_size);
        const T           invN = T{1} / static_cast<T>(N);

        // IFFT(X) = conj(FFT(conj(X))) / N
        for (std::size_t k = 0; k < N; ++k) {
            _work[k] = std::conj(inSpan[k]);
        }
        _fft.compute(_work, outSpan);
        for (std::size_t k = 0; k < N; ++k) {
            outSpan[k] = std::conj(outSpan[k]) * invN;
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fft

#endif // GNURADIO_IFFT_HPP
