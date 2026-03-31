#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/filter/HilbertTransform.hpp>

const boost::ut::suite<"HilbertTransform"> hilbertTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    "settingsChanged computes taps of correct length"_test = [] {
        HilbertTransform<float> block({{"n_taps", gr::Size_t{63U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(eq(block._taps.size(), std::size_t{63})) << "tap count matches n_taps";
        expect(approx(double(block._taps[31]), 0.0, 1e-9)) << "centre tap is zero";
        // even-offset taps (centre ± 2, ± 4, …) must be zero
        expect(approx(double(block._taps[29]), 0.0, 1e-9)) << "offset-2 tap is zero";
        expect(approx(double(block._taps[33]), 0.0, 1e-9)) << "offset+2 tap is zero";
        // odd-offset taps must be non-zero
        expect(neq(block._taps[30], 0.f)) << "offset-1 tap is non-zero";
        expect(neq(block._taps[32], 0.f)) << "offset+1 tap is non-zero";
    };

    "analytic signal of a sine has unit magnitude in steady state"_test = [] {
        // Generate a sine wave and check |out| ≈ amplitude after warm-up
        constexpr std::size_t kTaps     = 63U;
        constexpr std::size_t kWarmUp   = kTaps - 1U;
        constexpr std::size_t kCheckLen = 256U;
        constexpr float       kAmplitude = 2.0f;
        constexpr float       kFreq      = 0.1f; // normalised frequency (cycles per sample)
        constexpr float       kTwoPi     = static_cast<float>(2.0 * std::numbers::pi);

        HilbertTransform<float> block({{"n_taps", gr::Size_t{kTaps}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // warm up
        for (std::size_t i = 0; i < kWarmUp; ++i) {
            std::ignore = block.processOne(kAmplitude * std::sin(kTwoPi * kFreq * static_cast<float>(i)));
        }

        // check magnitude in steady state
        double maxErr = 0.0;
        for (std::size_t i = kWarmUp; i < kWarmUp + kCheckLen; ++i) {
            const std::complex<float> y = block.processOne(kAmplitude * std::sin(kTwoPi * kFreq * static_cast<float>(i)));
            maxErr = std::max(maxErr, std::abs(double(std::abs(y)) - double(kAmplitude)));
        }
        expect(lt(maxErr, 0.05)) << "steady-state magnitude ≈ amplitude (within 5 %)";
    };

    "analytic signal of a cosine has unit magnitude in steady state"_test = [] {
        constexpr std::size_t kTaps   = 63U;
        constexpr std::size_t kWarmUp = kTaps - 1U;
        constexpr float       kFreq   = 0.15f;
        constexpr float       kTwoPi  = static_cast<float>(2.0 * std::numbers::pi);

        HilbertTransform<float> block({{"n_taps", gr::Size_t{kTaps}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        for (std::size_t i = 0; i < kWarmUp; ++i) {
            std::ignore = block.processOne(std::cos(kTwoPi * kFreq * static_cast<float>(i)));
        }

        double maxErr = 0.0;
        for (std::size_t i = kWarmUp; i < kWarmUp + 128U; ++i) {
            const std::complex<float> y = block.processOne(std::cos(kTwoPi * kFreq * static_cast<float>(i)));
            maxErr = std::max(maxErr, std::abs(double(std::abs(y)) - 1.0));
        }
        expect(lt(maxErr, 0.05)) << "cosine analytic signal magnitude ≈ 1";
    };

    "double precision variant compiles and runs"_test = [] {
        HilbertTransform<double> block({{"n_taps", gr::Size_t{31U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const std::complex<double> y = block.processOne(1.0);
        expect(neq(y.real(), 0.0) || neq(y.imag(), 0.0)) << "double block produces output";
    };

    "settings change resets history and taps"_test = [] {
        HilbertTransform<float> block({{"n_taps", gr::Size_t{31U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime with non-zero data
        for (int i = 0; i < 31; ++i) {
            std::ignore = block.processOne(1.f);
        }

        // change tap count
        block.n_taps = gr::Size_t{63U};
        block.settingsChanged({}, {{"n_taps", gr::Size_t{63U}}});

        expect(eq(block._taps.size(), std::size_t{63})) << "taps resized";
        expect(eq(block._center, std::size_t{31})) << "centre updated";
        // after reset the first output should reflect a single sample (not old history)
        const std::complex<float> y = block.processOne(0.f);
        expect(approx(double(y.real()), 0.0, 1e-6)) << "real part after reset is zero (no input yet)";
    };
};

int main() { /* tests auto-register via boost::ut */ }
