#include <boost/ut.hpp>
#include <cmath>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/DataSet.hpp>
#include <gnuradio-4.0/fourier/SpectralEstimator.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"SpectralEstimator"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fft;

    "settingsChanged sets chunk sizes"_test = []<typename T> {
        SpectralEstimator<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size   = 64U;
        block.n_averages = 4U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{256}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{1}));
    } | std::tuple<float, double>{};

    "white noise estimate has flat spectrum"_test = [] {
        SpectralEstimator<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        constexpr std::size_t N   = 64;
        constexpr std::size_t nav = 1;
        block.fft_size   = static_cast<gr::Size_t>(N);
        block.n_averages = static_cast<gr::Size_t>(nav);
        block.window     = "Rectangular";
        block.settingsChanged({}, {});

        // Pure sine at bin 8 (frequency 8/N * sample_rate)
        const float freq = 8.f / static_cast<float>(N);
        std::vector<float> input(N * nav);
        for (std::size_t i = 0; i < input.size(); ++i) {
            input[i] = std::cos(2.f * std::numbers::pi_v<float> * freq * static_cast<float>(i));
        }

        std::vector<gr::DataSet<float>> ds(1);
        std::ignore = block.processBulk(std::span<const float>{input}, std::span<gr::DataSet<float>>{ds});

        expect(eq(ds[0].signal_values.size(), N)) << "output has N bins";
        // Bin 8 should have the highest power
        const auto& psd      = ds[0].signal_values;
        const float peakBin  = static_cast<float>(*std::max_element(psd.begin(), psd.end()));
        expect(peakBin > 0.1f) << "peak power > 0.1";
    };
};
