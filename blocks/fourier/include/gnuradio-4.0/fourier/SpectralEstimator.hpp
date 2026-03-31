#ifndef GNURADIO_SPECTRAL_ESTIMATOR_HPP
#define GNURADIO_SPECTRAL_ESTIMATOR_HPP

#include <complex>
#include <numeric>
#include <string>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/DataSet.hpp>
#include <gnuradio-4.0/algorithm/fourier/fft.hpp>
#include <gnuradio-4.0/algorithm/fourier/fft_common.hpp>
#include <gnuradio-4.0/algorithm/fourier/window.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fft {

GR_REGISTER_BLOCK("gr::blocks::fft::SpectralEstimator", gr::blocks::fft::SpectralEstimator, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct SpectralEstimator : gr::Block<SpectralEstimator<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Power spectral density estimate using Welch's (Bartlett) method.

Accumulates `n_averages` non-overlapping frames of `fft_size` samples each.
Each frame is windowed, FFT'd, and its power spectrum accumulated. After
`n_averages` frames the averaged power spectral density is emitted as a
`DataSet<T>`.
)"">;

    using complex_type = std::complex<T>;
    using DataSetT     = DataSet<T>;

    PortIn<T>                              in;
    PortOut<DataSetT, RequiredSamples<1U>> out;

    Annotated<gr::Size_t,  "fft_size",   Doc<"samples per FFT frame">, Visible>           fft_size{1024U};
    Annotated<gr::Size_t,  "n_averages", Doc<"frames to average per output DataSet">>      n_averages{8U};
    Annotated<std::string, "window",     Doc<"window function name (e.g. Hann, Hamming)">> window{"Hann"};
    Annotated<float,       "sample_rate", Doc<"signal sample rate">, Unit<"Hz">>           sample_rate{1.f};

    GR_MAKE_REFLECTABLE(SpectralEstimator, in, out, fft_size, n_averages, window, sample_rate);

    gr::algorithm::FFT<complex_type, complex_type> _fft{};
    std::vector<T>                                 _windowCoeffs{};
    std::vector<T>                                 _accumPsd{};
    std::vector<complex_type>                      _spectrum{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N   = static_cast<std::size_t>(fft_size);
        const std::size_t nav = static_cast<std::size_t>(n_averages);

        this->input_chunk_size  = static_cast<gr::Size_t>(N * nav);
        this->output_chunk_size = 1U;

        auto wt = gr::algorithm::window::Type::Hann;
        if (const auto parsed = magic_enum::enum_cast<gr::algorithm::window::Type>(window, magic_enum::case_insensitive)) {
            wt = *parsed;
        }
        _windowCoeffs = gr::algorithm::window::create<T>(wt, N);
        _accumPsd.assign(N, T{});
        _spectrum.resize(N);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<DataSetT> outSpan) noexcept {
        const std::size_t N   = static_cast<std::size_t>(fft_size);
        const std::size_t nav = static_cast<std::size_t>(n_averages);
        const T           invN = T{1} / static_cast<T>(N);

        std::fill(_accumPsd.begin(), _accumPsd.end(), T{});

        std::vector<complex_type> frame(N);
        for (std::size_t av = 0; av < nav; ++av) {
            const std::size_t offset = av * N;
            for (std::size_t i = 0; i < N; ++i) {
                frame[i] = complex_type{inSpan[offset + i] * _windowCoeffs[i]};
            }
            _fft.compute(frame, _spectrum);
            for (std::size_t k = 0; k < N; ++k) {
                const T re = _spectrum[k].real() * invN;
                const T im = _spectrum[k].imag() * invN;
                _accumPsd[k] += re * re + im * im;
            }
        }

        const T invNav = T{1} / static_cast<T>(nav);
        for (auto& v : _accumPsd) v *= invNav;

        DataSetT& ds = outSpan[0];
        ds           = DataSetT{};
        ds.extents   = {static_cast<int32_t>(N)};
        ds.layout    = gr::LayoutRight{};

        ds.axis_names  = {"Frequency"};
        ds.axis_units  = {"Hz"};
        ds.axis_values.resize(1UZ);
        ds.axis_values[0].resize(N);
        const T freqWidth = static_cast<T>(sample_rate) / static_cast<T>(N);
        for (std::size_t k = 0; k < N; ++k) {
            ds.axis_values[0][k] = static_cast<T>(k) * freqWidth;
        }

        ds.signal_names  = {"PSD"};
        ds.signal_units  = {"a.u."};
        ds.signal_values.resize(N);
        std::ranges::copy(_accumPsd, ds.signal_values.begin());

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fft

#endif // GNURADIO_SPECTRAL_ESTIMATOR_HPP
