#include <boost/ut.hpp>
#include <cmath>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/electrical/GridFrequencyEstimator.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"GridFrequencyEstimator"> tests = [] {
    using namespace boost::ut;
    using namespace gr::electrical;

    "starts at nominal frequency"_test = []<typename T> {
        GridFrequencyEstimator<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.sample_rate  = static_cast<T>(10000);
        block.nominal_freq = static_cast<T>(50);
        block.settingsChanged({}, {});

        expect(approx(static_cast<double>(block._freqEstimate), 50.0, 1e-6));
    } | std::tuple<float, double>{};

    "settingsChanged resets state"_test = [] {
        GridFrequencyEstimator<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.sample_rate  = 10000.f;
        block.nominal_freq = 60.f;
        block.settingsChanged({}, {});

        expect(approx(static_cast<double>(block._freqEstimate), 60.0, 1e-6));
        expect(eq(block._samplesSinceCrossing, std::size_t{0}));
    };

    "detects 50 Hz from 200 samples per period at 10 kHz"_test = [] {
        // 50 Hz at 10 kHz = 200 samples per cycle
        constexpr float fs = 10000.f;
        constexpr float f0 = 50.f;
        constexpr std::size_t nCycles = 3;
        constexpr std::size_t N       = static_cast<std::size_t>(fs / f0) * nCycles;

        GridFrequencyEstimator<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.sample_rate  = fs;
        block.nominal_freq = f0;
        block.settingsChanged({}, {});

        float lastOut = 0.f;
        for (std::size_t n = 0; n < N; ++n) {
            const float x = std::sin(2.f * std::numbers::pi_v<float> * f0 * static_cast<float>(n) / fs);
            lastOut = block.processOne(x);
        }
        expect(approx(static_cast<double>(lastOut), 50.0, 1.0)) << "frequency estimate near 50 Hz";
    };
};
