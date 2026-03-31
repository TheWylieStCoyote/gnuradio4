#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/math/AutoCorrelation.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"AutoCorrelation"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    auto makeBlock = []<typename T>(gr::Size_t wsize, gr::Size_t mlag) {
        AutoCorrelation<T> block({});
        block.settings().init();
        std::ignore       = block.settings().applyStagedParameters();
        block.window_size = wsize;
        block.max_lag     = mlag;
        block.settingsChanged({}, {});
        return block;
    };

    "lag-0 equals mean power for DC signal"_test = [&makeBlock]<typename T> {
        // DC signal of amplitude 2 → r[0] = 4
        constexpr gr::Size_t kW = 8U;
        constexpr gr::Size_t kL = 3U;
        auto block = makeBlock.template operator()<T>(kW, kL);

        std::vector<T> inBuf(kW, T{2});
        std::vector<T> outBuf(kL + 1U, T{});
        std::ignore = block.processBulk(std::span{inBuf}, std::span{outBuf});

        expect(approx(static_cast<double>(outBuf[0]), 4.0, 1e-4)) << "r[0] = mean power";
        // biased estimator: r[k] = (W-k)/W * A² for DC
        for (std::size_t k = 1; k <= kL; ++k) {
            const double expected = (static_cast<double>(kW) - static_cast<double>(k)) / static_cast<double>(kW) * 4.0;
            expect(approx(static_cast<double>(outBuf[k]), expected, 1e-4)) << "r[k] biased DC";
        }
    } | std::tuple<float, double>{};

    "lag-0 is real and largest for random signal"_test = [] {
        constexpr gr::Size_t kW = 64U;
        constexpr gr::Size_t kL = 8U;
        auto block = [kW, kL] {
            AutoCorrelation<float> b({});
            b.settings().init();
            std::ignore     = b.settings().applyStagedParameters();
            b.window_size   = kW;
            b.max_lag       = kL;
            b.settingsChanged({}, {});
            return b;
        }();

        // pseudo-random signal
        std::vector<float> inBuf(kW);
        for (std::size_t i = 0; i < kW; ++i) {
            inBuf[i] = static_cast<float>((i * 7 + 3) % 11) - 5.0f;
        }
        std::vector<float> outBuf(kL + 1U);
        std::ignore = block.processBulk(std::span{inBuf}, std::span{outBuf});

        for (std::size_t k = 1; k <= kL; ++k) {
            expect(outBuf[0] >= std::abs(outBuf[k])) << "r[0] >= |r[k]| (Wiener-Khinchin)";
        }
    };

    "complex signal: lag-0 equals mean power"_test = [] {
        using CT = std::complex<float>;
        constexpr gr::Size_t kW = 16U;
        constexpr gr::Size_t kL = 4U;
        AutoCorrelation<CT> block({});
        block.settings().init();
        std::ignore     = block.settings().applyStagedParameters();
        block.window_size = kW;
        block.max_lag     = kL;
        block.settingsChanged({}, {});

        std::vector<CT> inBuf(kW, CT{1.f, 1.f}); // |x|² = 2 per sample
        std::vector<CT> outBuf(kL + 1U, CT{});
        std::ignore = block.processBulk(std::span{inBuf}, std::span{outBuf});

        // r[0] = mean(|x|²) = 2, purely real for a constant signal
        expect(approx(static_cast<double>(outBuf[0].real()), 2.0, 1e-4));
        expect(approx(static_cast<double>(outBuf[0].imag()), 0.0, 1e-4));
    };

    "settingsChanged resets block_size parameters"_test = [] {
        AutoCorrelation<float> block({});
        block.settings().init();
        std::ignore     = block.settings().applyStagedParameters();
        block.window_size = 32U;
        block.max_lag     = 8U;
        block.settingsChanged({}, {});
        expect(eq(block.input_chunk_size, gr::Size_t{32U}));
        expect(eq(block.output_chunk_size, gr::Size_t{9U})); // max_lag + 1
    };
};
