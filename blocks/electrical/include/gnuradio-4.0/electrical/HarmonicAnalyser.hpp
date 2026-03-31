#ifndef GNURADIO_HARMONIC_ANALYSER_HPP
#define GNURADIO_HARMONIC_ANALYSER_HPP

#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/DataSet.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::electrical {

GR_REGISTER_BLOCK(gr::electrical::HarmonicAnalyser, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct HarmonicAnalyser : gr::Block<HarmonicAnalyser<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Estimate amplitudes and phases at the fundamental frequency and its harmonics.

Uses the Goertzel algorithm for each harmonic. Produces one `DataSet<T>` per block
of `block_size` samples. The DataSet contains `n_harmonics` entries: amplitude and phase
for harmonics 1, 2, …, n_harmonics.
)"">;

    using DataSetT = DataSet<T>;

    PortIn<T>                              in;
    PortOut<DataSetT, RequiredSamples<1U>> out;

    Annotated<gr::Size_t, "block_size",    Doc<"samples per analysis frame">, Visible>   block_size{1024U};
    Annotated<T,          "fundamental",   Doc<"fundamental frequency">, Unit<"Hz">>     fundamental{50};
    Annotated<T,          "sample_rate",   Doc<"signal sample rate">, Unit<"Hz">>        sample_rate{10000};
    Annotated<gr::Size_t, "n_harmonics",   Doc<"number of harmonics to analyse">>        n_harmonics{5U};

    GR_MAKE_REFLECTABLE(HarmonicAnalyser, in, out, block_size, fundamental, sample_rate, n_harmonics);

    struct GoertzelState {
        T coeff{};
        T cosW{};
        T sinW{};
    };
    std::vector<GoertzelState> _states{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N    = static_cast<std::size_t>(block_size);
        const std::size_t nH   = static_cast<std::size_t>(n_harmonics);
        this->input_chunk_size  = static_cast<gr::Size_t>(N);
        this->output_chunk_size = 1U;

        _states.resize(nH);
        for (std::size_t h = 0; h < nH; ++h) {
            const T W         = T{2} * std::numbers::pi_v<T> * static_cast<T>(h + 1UZ) * static_cast<T>(fundamental) / static_cast<T>(sample_rate);
            _states[h].coeff  = T{2} * std::cos(W);
            _states[h].cosW   = std::cos(W);
            _states[h].sinW   = std::sin(W);
        }
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<DataSetT> outSpan) noexcept {
        const std::size_t nH = _states.size();

        std::vector<T> s1(nH, T{});
        std::vector<T> s2(nH, T{});

        for (const T x : inSpan) {
            for (std::size_t h = 0; h < nH; ++h) {
                const T s0 = x + _states[h].coeff * s1[h] - s2[h];
                s2[h]      = s1[h];
                s1[h]      = s0;
            }
        }

        DataSetT& ds = outSpan[0];
        ds           = DataSetT{};
        ds.extents   = {static_cast<int32_t>(nH)};
        ds.layout    = gr::LayoutRight{};

        ds.axis_names  = {"Harmonic"};
        ds.axis_units  = {""};
        ds.axis_values.resize(1UZ);
        ds.axis_values[0].resize(nH);
        for (std::size_t h = 0; h < nH; ++h) {
            ds.axis_values[0][h] = static_cast<T>(h + 1UZ);
        }

        ds.signal_names  = {"Amplitude", "Phase"};
        ds.signal_units  = {"a.u.", "rad"};
        ds.signal_values.resize(2UZ * nH);

        for (std::size_t h = 0; h < nH; ++h) {
            const T re = s1[h] - s2[h] * _states[h].cosW;
            const T im = s2[h] * _states[h].sinW;
            const T N  = static_cast<T>(static_cast<std::size_t>(block_size));
            ds.signal_values[h]      = std::sqrt(re * re + im * im) * T{2} / N;
            ds.signal_values[nH + h] = std::atan2(im, re);
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::electrical

#endif // GNURADIO_HARMONIC_ANALYSER_HPP
