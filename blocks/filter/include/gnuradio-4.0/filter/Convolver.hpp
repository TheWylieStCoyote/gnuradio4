#ifndef GNURADIO_CONVOLVER_HPP
#define GNURADIO_CONVOLVER_HPP

#include <bit>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/fourier/fft.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::Convolver, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Convolver : gr::Block<Convolver<T>, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Overlap-add FFT convolver.

Convolves a streaming signal with a fixed FIR `kernel` using the overlap-add
method. The FFT frame size is the next power of two ≥ `block_size + len(kernel) − 1`.
The `kernel` holds the real-valued impulse response; for complex input signals the
same kernel is applied to each complex component via the conjugate-symmetry IFFT trick.
Output lag relative to direct convolution is zero; the overlap buffer carries inter-block
state automatically.
)"">;

    using value_type   = decltype(std::real(std::declval<T>()));
    using complex_type = std::complex<value_type>;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t,             "block_size", Doc<"input samples per processing frame (power of two recommended)">, Visible> block_size{512U};
    Annotated<std::vector<value_type>, "kernel",    Doc<"real-valued FIR impulse response coefficients">>                          kernel{std::vector<value_type>{value_type{1}}};

    GR_MAKE_REFLECTABLE(Convolver, in, out, block_size, kernel);

    gr::algorithm::FFT<complex_type, complex_type> _fft{};
    std::vector<complex_type>                      _kernelSpectrum{};
    std::vector<complex_type>                      _overlapBuf{};
    std::vector<complex_type>                      _workBuf{};
    std::size_t                                    _fftSize{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t              L   = static_cast<std::size_t>(block_size);
        const std::vector<value_type>& h   = kernel.value;
        const std::size_t              P   = h.empty() ? 1UZ : h.size();

        _fftSize = std::bit_ceil(L + P - 1UZ);
        this->input_chunk_size  = L;
        this->output_chunk_size = L;

        std::vector<complex_type> kernelPad(_fftSize, complex_type{});
        for (std::size_t i = 0; i < h.size(); ++i) {
            kernelPad[i] = complex_type{h[i]};
        }
        _kernelSpectrum.resize(_fftSize);
        _fft.compute(kernelPad, _kernelSpectrum);

        _overlapBuf.assign(P - 1UZ, complex_type{});
        _workBuf.resize(_fftSize);
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t L          = static_cast<std::size_t>(block_size);
        const std::size_t overlapLen = _overlapBuf.size();

        if (_fftSize == 0UZ) {
            std::ranges::copy(inSpan, outSpan.begin());
            return work::Status::OK;
        }

        // Fill complex work buffer from input; zero-pad to fft size
        std::fill(_workBuf.begin(), _workBuf.end(), complex_type{});
        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            _workBuf[i] = complex_type{static_cast<value_type>(std::real(inSpan[i])),
                                       static_cast<value_type>(std::imag(inSpan[i]))};
        }

        // Forward FFT in-place (reuse _workBuf as output)
        std::vector<complex_type> X(_fftSize);
        _fft.compute(_workBuf, X);

        // Multiply spectra
        for (std::size_t k = 0; k < _fftSize; ++k) {
            X[k] *= _kernelSpectrum[k];
        }

        // IFFT via conjugate trick: IFFT(X) = conj(FFT(conj(X))) / N
        for (auto& v : X) v = std::conj(v);
        std::vector<complex_type> y(_fftSize);
        _fft.compute(X, y);
        const value_type invN = value_type{1} / static_cast<value_type>(_fftSize);
        for (auto& v : y) v = std::conj(v) * invN;

        // Overlap-add
        for (std::size_t i = 0; i < overlapLen; ++i) {
            y[i] += _overlapBuf[i];
        }

        // Copy output
        for (std::size_t i = 0; i < L; ++i) {
            if constexpr (gr::meta::complex_like<T>) {
                outSpan[i] = T{static_cast<value_type>(y[i].real()), static_cast<value_type>(y[i].imag())};
            } else {
                outSpan[i] = static_cast<T>(y[i].real());
            }
        }

        // Save new overlap for next frame
        for (std::size_t i = 0; i < overlapLen; ++i) {
            _overlapBuf[i] = y[L + i];
        }

        return work::Status::OK;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_CONVOLVER_HPP
