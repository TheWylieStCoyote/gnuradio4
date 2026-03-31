#include <boost/ut.hpp>
#include <cmath>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/DataSet.hpp>
#include <gnuradio-4.0/electrical/HarmonicAnalyser.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"HarmonicAnalyser"> tests = [] {
    using namespace boost::ut;
    using namespace gr::electrical;

    "settingsChanged sets chunk sizes"_test = []<typename T> {
        HarmonicAnalyser<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.block_size  = 256U;
        block.n_harmonics = 3U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{256}));
        expect(eq(block._states.size(), std::size_t{3}));
    } | std::tuple<float, double>{};

    "pure sine produces one dominant harmonic"_test = [] {
        constexpr std::size_t N  = 512;
        constexpr float       fs = 10000.f;
        constexpr float       f0 = 50.f;

        HarmonicAnalyser<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.block_size  = static_cast<gr::Size_t>(N);
        block.fundamental = f0;
        block.sample_rate = fs;
        block.n_harmonics = 3U;
        block.settingsChanged({}, {});

        std::vector<float> input(N);
        for (std::size_t n = 0; n < N; ++n) {
            input[n] = std::cos(2.f * std::numbers::pi_v<float> * f0 * static_cast<float>(n) / fs);
        }

        std::vector<gr::DataSet<float>> ds(1);
        std::ignore = block.processBulk(std::span<const float>{input}, std::span<gr::DataSet<float>>{ds});

        expect(eq(ds[0].signal_values.size(), std::size_t{6})) << "3 amps + 3 phases";
        // First harmonic amplitude should be dominant
        expect(ds[0].signal_values[0] > ds[0].signal_values[1]) << "fundamental > 2nd harmonic";
    };
};
