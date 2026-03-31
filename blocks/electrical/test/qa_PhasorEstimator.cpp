#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/electrical/PhasorEstimator.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"PhasorEstimator"> tests = [] {
    using namespace boost::ut;
    using namespace gr::electrical;

    "settingsChanged updates chunk sizes"_test = []<typename T> {
        PhasorEstimator<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.block_size  = 512U;
        block.target_freq = static_cast<T>(50);
        block.sample_rate = static_cast<T>(10000);
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{512}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{1}));
    } | std::tuple<float, double>{};

    "pure sine at target frequency has nonzero amplitude"_test = [] {
        constexpr std::size_t N   = 256;
        constexpr float       fs  = 10000.f;
        constexpr float       f0  = 50.f;

        PhasorEstimator<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.block_size  = static_cast<gr::Size_t>(N);
        block.target_freq = f0;
        block.sample_rate = fs;
        block.settingsChanged({}, {});

        std::vector<float> input(N);
        for (std::size_t n = 0; n < N; ++n) {
            input[n] = std::cos(2.f * std::numbers::pi_v<float> * f0 * static_cast<float>(n) / fs);
        }

        std::vector<std::complex<float>> output(1);
        std::ignore = block.processBulk(std::span<const float>{input}, std::span<std::complex<float>>{output});

        const float amp = std::abs(output[0]) / static_cast<float>(N) * 2.f;
        expect(amp > 0.5f) << "amplitude of 50 Hz sine should be significant";
    };
};
