#include <boost/ut.hpp>
#include <cmath>
#include <numbers>
#include <random>
#include <vector>

#include <gnuradio-4.0/filter/WienerFilter.hpp>

int main() { /* tests auto-register via boost::ut */ }

namespace {

template<typename T>
auto makeBlock(gr::Size_t nTaps, gr::Size_t trainLen, float reg = 1e-6f) {
    gr::blocks::filter::WienerFilter<T> b({{"n_taps", nTaps}, {"training_length", trainLen}, {"regularisation", reg}});
    b.settings().init();
    std::ignore = b.settings().applyStagedParameters();
    return b;
}

} // namespace

const boost::ut::suite<"WienerFilter"> wienerTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    "output is zero during the training phase"_test = [] {
        constexpr std::size_t kTrain = 50UZ;
        auto                  block  = makeBlock<float>(gr::Size_t{4U}, gr::Size_t{kTrain});

        std::vector<float> in(kTrain), desired(kTrain), out(kTrain);
        for (std::size_t i = 0; i < kTrain; ++i) { in[i] = static_cast<float>(i + 1); }
        desired = in;

        std::ignore = block.processBulk(std::span<const float>(in), std::span<const float>(desired), std::span<float>(out));

        for (std::size_t i = 0; i < kTrain; ++i) {
            expect(eq(out[i], 0.f)) << "training output must be zero at sample " << i;
        }

        // one more sample — filter is now active
        std::vector<float> in2{1.f}, des2{1.f}, out2{0.f};
        std::ignore = block.processBulk(std::span<const float>(in2), std::span<const float>(des2), std::span<float>(out2));
        expect(neq(out2[0], 0.f)) << "first post-training output must be non-zero";
    };

    "settingsChanged resets training state"_test = [] {
        auto block = makeBlock<float>(gr::Size_t{4U}, gr::Size_t{100U});

        // partial training: 20 samples
        std::vector<float> in(20, 1.f), des(20, 1.f), out(20);
        std::ignore = block.processBulk(std::span<const float>(in), std::span<const float>(des), std::span<float>(out));

        expect(eq(block._samplesAccumulated, std::size_t{20}));

        block.n_taps = gr::Size_t{8U};
        block.settingsChanged({}, {{"n_taps", gr::Size_t{8U}}});

        expect(eq(block._samplesAccumulated, std::size_t{0})) << "_samplesAccumulated reset";
        expect(eq(block._trained, false)) << "_trained reset";
        expect(eq(block._taps.size(), std::size_t{8})) << "taps vector resized";
    };

    "identity recovery: desired = in, taps converge to delta"_test = [] {
        constexpr std::size_t kTrain = 400UZ;
        const float           kPi    = std::numbers::pi_v<float>;
        auto                  block  = makeBlock<float>(gr::Size_t{4U}, gr::Size_t{kTrain});

        std::vector<float> in(kTrain), desired(kTrain), out(kTrain);
        for (std::size_t n = 0; n < kTrain; ++n) {
            in[n] = std::cos(static_cast<float>(2.0) * kPi * static_cast<float>(0.07) * static_cast<float>(n))
                  + std::cos(static_cast<float>(2.0) * kPi * static_cast<float>(0.13) * static_cast<float>(n));
        }
        desired = in;

        std::ignore = block.processBulk(std::span<const float>(in), std::span<const float>(desired), std::span<float>(out));

        expect(block._trained) << "block must be trained after kTrain samples";
        expect(lt(std::abs(static_cast<double>(block._taps[0]) - 1.0), 0.05)) << "taps[0] ≈ 1";
        expect(lt(std::abs(static_cast<double>(block._taps[1])), 0.05)) << "taps[1] ≈ 0";
        expect(lt(std::abs(static_cast<double>(block._taps[2])), 0.05)) << "taps[2] ≈ 0";
        expect(lt(std::abs(static_cast<double>(block._taps[3])), 0.05)) << "taps[3] ≈ 0";
    };

    "delay identification: desired = in delayed by 3 samples"_test = [] {
        constexpr std::size_t kTrain = 1000UZ;
        constexpr std::size_t kDelay = 3UZ;
        auto                  block  = makeBlock<float>(gr::Size_t{8U}, gr::Size_t{kTrain}, 1e-7f);

        std::vector<float> in(kTrain), desired(kTrain), out(kTrain);
        std::mt19937                     rng{42U};
        std::uniform_real_distribution<float> dist{-1.f, 1.f};
        for (std::size_t n = 0; n < kTrain; ++n) { in[n] = dist(rng); }
        for (std::size_t n = 0; n < kTrain; ++n) {
            desired[n] = n >= kDelay ? in[n - kDelay] : 0.f;
        }

        std::ignore = block.processBulk(std::span<const float>(in), std::span<const float>(desired), std::span<float>(out));

        expect(block._trained) << "block must be trained";

        // tap 3 should be the dominant tap
        float maxMag  = 0.f;
        std::size_t maxIdx = 0UZ;
        for (std::size_t k = 0; k < 8UZ; ++k) {
            if (std::abs(block._taps[k]) > maxMag) {
                maxMag = std::abs(block._taps[k]);
                maxIdx = k;
            }
        }
        expect(eq(maxIdx, kDelay)) << "dominant tap is at the correct delay index";
        expect(gt(static_cast<double>(std::abs(block._taps[kDelay])), 0.7)) << "tap[3] magnitude > 0.7";
        for (std::size_t k = 0; k < 8UZ; ++k) {
            if (k != kDelay) {
                expect(lt(static_cast<double>(std::abs(block._taps[k])), 0.2))
                    << "tap[" << k << "] magnitude < 0.2";
            }
        }
    };

    "noise reduction: output MSE less than half input MSE"_test = [] {
        constexpr std::size_t kTrain   = 2000UZ;
        constexpr std::size_t kTest    = 500UZ;
        constexpr std::size_t kWarmUp  = 100UZ;
        const float           kPi      = std::numbers::pi_v<float>;
        auto                  block    = makeBlock<float>(gr::Size_t{8U}, gr::Size_t{kTrain});

        auto makeSamples = [&](std::size_t n, float noiseStd, std::uint32_t seed) {
            std::mt19937                         rng{seed};
            std::normal_distribution<float>      noise{0.f, noiseStd};
            std::vector<float> sig(n), noisyIn(n), des(n);
            for (std::size_t i = 0; i < n; ++i) {
                sig[i]     = std::cos(static_cast<float>(2.0) * kPi * static_cast<float>(0.1) * static_cast<float>(i));
                noisyIn[i] = sig[i] + noise(rng);
                des[i]     = sig[i];
            }
            return std::tuple{sig, noisyIn, des};
        };

        // training phase
        auto [trainSig, trainIn, trainDes] = makeSamples(kTrain, 0.5f, 123U);
        std::vector<float> trainOut(kTrain);
        std::ignore = block.processBulk(std::span<const float>(trainIn), std::span<const float>(trainDes), std::span<float>(trainOut));

        // test phase
        auto [testSig, testIn, testDes] = makeSamples(kTest, 0.5f, 999U);
        std::vector<float> testOut(kTest);
        std::ignore = block.processBulk(std::span<const float>(testIn), std::span<const float>(testDes), std::span<float>(testOut));

        double inputMse  = 0.0;
        double outputMse = 0.0;
        for (std::size_t i = kWarmUp; i < kTest; ++i) {
            const double inErr  = static_cast<double>(testIn[i]) - static_cast<double>(testSig[i]);
            const double outErr = static_cast<double>(testOut[i]) - static_cast<double>(testSig[i]);
            inputMse  += inErr * inErr;
            outputMse += outErr * outErr;
        }
        inputMse  /= static_cast<double>(kTest - kWarmUp);
        outputMse /= static_cast<double>(kTest - kWarmUp);

        expect(lt(outputMse, 0.5 * inputMse))
            << "output MSE (" << outputMse << ") must be < 0.5 * input MSE (" << inputMse << ")";
    };
};
