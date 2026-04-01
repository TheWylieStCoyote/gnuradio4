#ifndef GNURADIO_SPECTRAL_SUBTRACTOR_HPP
#define GNURADIO_SPECTRAL_SUBTRACTOR_HPP

#include <bit>
#include <cmath>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/fourier/fft.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fourier {

GR_REGISTER_BLOCK(gr::blocks::fourier::SpectralSubtractor, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct SpectralSubtractor : gr::Block<SpectralSubtractor<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Frequency-domain noise reduction via spectral subtraction.

Estimates the noise floor from the first `reference_frames` input frames (which should
contain noise only), then subtracts `alpha` × noise RMS from each bin magnitude in
subsequent frames. Clamped to zero to avoid negative magnitudes. Frame size is `fft_size`;
input and output chunk sizes match `fft_size`.
)"">;

    using value_type   = decltype(std::real(std::declval<T>()));
    using complex_type = std::complex<value_type>;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "fft_size",         Doc<"frame size in samples (power of two recommended)">, Visible> fft_size{512U};
    Annotated<value_type, "alpha",             Doc<"over-subtraction factor (≥ 1 = full subtraction)">, Visible> alpha{static_cast<value_type>(1)};
    Annotated<gr::Size_t, "reference_frames",  Doc<"noise-only frames used to estimate the noise floor">>         reference_frames{10U};

    GR_MAKE_REFLECTABLE(SpectralSubtractor, in, out, fft_size, alpha, reference_frames);

    gr::algorithm::FFT<complex_type, complex_type> _fft{};
    std::vector<value_type>   _noiseFloor{};
    std::vector<value_type>   _noiseAccum{};
    std::vector<complex_type> _workBuf{};
    std::vector<complex_type> _spectrum{};
    std::vector<complex_type> _ifftBuf{};
    std::size_t               _frameCount{0};
    std::size_t               _fftN{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(fft_size);
        _fftN = std::bit_ceil(N);
        _noiseFloor.assign(_fftN, value_type{});
        _noiseAccum.assign(_fftN, value_type{});
        _workBuf.resize(_fftN);
        _spectrum.resize(_fftN);
        _ifftBuf.resize(_fftN);
        _frameCount = 0UZ;
        this->input_chunk_size  = static_cast<gr::Size_t>(N);
        this->output_chunk_size = static_cast<gr::Size_t>(N);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t N = static_cast<std::size_t>(fft_size);
        const std::size_t R = static_cast<std::size_t>(reference_frames);

        // load input into zero-padded complex work buffer
        std::fill(_workBuf.begin(), _workBuf.end(), complex_type{});
        for (std::size_t i = 0; i < N; ++i) {
            _workBuf[i] = complex_type{static_cast<value_type>(std::real(inSpan[i])),
                                        static_cast<value_type>(std::imag(inSpan[i]))};
        }

        _fft.compute(_workBuf, _spectrum);

        if (_frameCount < R) {
            for (std::size_t k = 0; k < _fftN; ++k) {
                _noiseAccum[k] += std::norm(_spectrum[k]); // |z|² = real² + imag²
            }
            if (++_frameCount == R) {
                const value_type invR = value_type{1} / static_cast<value_type>(R);
                for (std::size_t k = 0; k < _fftN; ++k) {
                    _noiseFloor[k] = _noiseAccum[k] * invR;
                }
            }
            // pass through unchanged during reference phase
            std::ranges::copy(inSpan, outSpan.begin());
            return gr::work::Status::OK;
        }

        // spectral subtraction: reduce each bin magnitude by alpha * noise RMS
        const value_type a = static_cast<value_type>(alpha);
        for (std::size_t k = 0; k < _fftN; ++k) {
            const value_type mag      = std::abs(_spectrum[k]);
            const value_type noiseRms = std::sqrt(_noiseFloor[k]);
            const value_type cleanMag = std::max(mag - a * noiseRms, value_type{});
            _spectrum[k] = (mag > value_type{}) ? (_spectrum[k] * (cleanMag / mag)) : complex_type{};
        }

        // IFFT via conjugate trick: IFFT(X) = conj(FFT(conj(X))) / N
        for (auto& v : _spectrum) v = std::conj(v);
        _fft.compute(_spectrum, _ifftBuf);
        const value_type invN = value_type{1} / static_cast<value_type>(_fftN);
        for (auto& v : _ifftBuf) v = std::conj(v) * invN;

        for (std::size_t i = 0; i < N; ++i) {
            if constexpr (gr::meta::complex_like<T>) {
                outSpan[i] = T{static_cast<value_type>(_ifftBuf[i].real()),
                               static_cast<value_type>(_ifftBuf[i].imag())};
            } else {
                outSpan[i] = static_cast<T>(_ifftBuf[i].real());
            }
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fourier

#endif // GNURADIO_SPECTRAL_SUBTRACTOR_HPP
