#include <boost/ut.hpp>
#include <cmath>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/math/Goertzel.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Goertzel"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    "detects pure tone at target frequency"_test = []<typename T> {
        constexpr T       kFs    = T{8000};
        constexpr T       kFreq  = T{1000};
        constexpr gr::Size_t kN  = 256U;

        Goertzel<T> block({});
        block.settings().init();
        std::ignore          = block.settings().applyStagedParameters();
        block.target_frequency = kFreq;
        block.sample_rate      = kFs;
        block.block_size       = kN;
        block.settingsChanged({}, {});

        std::vector<T> inBuf(kN);
        for (std::size_t i = 0; i < kN; ++i) {
            inBuf[i] = std::sin(T{2} * std::numbers::pi_v<T> * kFreq / kFs * static_cast<T>(i));
        }
        std::vector<T> outBuf(1, T{});
        std::ignore = block.processBulk(std::span{inBuf}, std::span{outBuf});

        expect(outBuf[0] > static_cast<T>(100)) << "strong magnitude at target freq";
    } | std::tuple<float, double>{};

    "off-frequency tone gives small magnitude"_test = [] {
        constexpr float kFs    = 8000.f;
        constexpr float kFreq  = 1000.f;
        constexpr float kOther = 3000.f;
        constexpr gr::Size_t kN = 256U;

        Goertzel<float> block({});
        block.settings().init();
        std::ignore          = block.settings().applyStagedParameters();
        block.target_frequency = kFreq;
        block.sample_rate      = kFs;
        block.block_size       = kN;
        block.settingsChanged({}, {});

        std::vector<float> inBuf(kN);
        for (std::size_t i = 0; i < kN; ++i) {
            inBuf[i] = std::sin(2.f * std::numbers::pi_v<float> * kOther / kFs * static_cast<float>(i));
        }
        std::vector<float> outBuf(1, 0.f);
        std::ignore = block.processBulk(std::span{inBuf}, std::span{outBuf});

        // off-frequency signal → magnitude near zero
        expect(outBuf[0] < 5.f) << "low magnitude for off-frequency tone";
    };

    "settingsChanged sets chunk sizes"_test = [] {
        Goertzel<float> block({});
        block.settings().init();
        std::ignore          = block.settings().applyStagedParameters();
        block.target_frequency = 1000.f;
        block.sample_rate      = 8000.f;
        block.block_size       = 128U;
        block.settingsChanged({}, {});
        expect(eq(block.input_chunk_size, gr::Size_t{128U}));
        expect(eq(block.output_chunk_size, gr::Size_t{1U}));
    };
};
