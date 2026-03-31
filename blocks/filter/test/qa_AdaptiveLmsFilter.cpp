#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/filter/AdaptiveLmsFilter.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"AdaptiveLmsFilter"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    auto makeBlock = []<typename T>(std::size_t nTaps, typename AdaptiveLmsFilter<T>::value_type mu) {
        AdaptiveLmsFilter<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_taps    = static_cast<gr::Size_t>(nTaps);
        block.step_size = mu;
        block.settingsChanged({}, {});
        return block;
    };

    "error converges to zero for a static desired signal"_test = [&makeBlock]<typename T> {
        // desired = constant 1.0, input = constant 1.0 → filter should converge to h[0]≈1
        auto block = makeBlock.template operator()<T>(4, static_cast<typename AdaptiveLmsFilter<T>::value_type>(0.1));

        std::vector<T> outBuf(1), errBuf(1);
        typename AdaptiveLmsFilter<T>::value_type lastError = static_cast<typename AdaptiveLmsFilter<T>::value_type>(1);

        for (int iter = 0; iter < 200; ++iter) {
            const std::array<T, 1> x{T{1}};
            const std::array<T, 1> d{T{1}};
            std::ignore = block.processBulk(std::span<const T>{x}, std::span<const T>{d},
                                            std::span<T>{outBuf}, std::span<T>{errBuf});
            lastError = static_cast<typename AdaptiveLmsFilter<T>::value_type>(std::abs(std::real(errBuf[0])));
        }
        expect(lastError < static_cast<typename AdaptiveLmsFilter<T>::value_type>(0.01)) << "error converged";
    } | std::tuple<float, double>{};

    "output approaches desired after convergence"_test = [&makeBlock] {
        // desired = 2·in → filter should converge h[0] ≈ 2
        auto block = makeBlock.template operator()<double>(1, 0.1);

        std::vector<double> outBuf(1), errBuf(1);
        for (int iter = 0; iter < 500; ++iter) {
            const std::array<double, 1> x{1.0};
            const std::array<double, 1> d{2.0};
            std::ignore = block.processBulk(std::span{x}, std::span{d}, std::span{outBuf}, std::span{errBuf});
        }
        expect(approx(outBuf[0], 2.0, 0.05)) << "output ≈ desired";
    };

    "settingsChanged resets taps to zero"_test = [&makeBlock] {
        auto block = makeBlock.template operator()<float>(8, 0.01f);

        // run some samples to dirty the taps
        for (int i = 0; i < 20; ++i) {
            const std::array<float, 1> x{1.f}, d{1.f};
            std::array<float, 1>       o{}, e{};
            std::ignore = block.processBulk(std::span{x}, std::span{d}, std::span{o}, std::span{e});
        }

        block.settingsChanged({}, {});
        for (const auto& tap : block._taps) {
            expect(eq(tap, 0.f)) << "tap reset to zero";
        }
    };

    "leak_factor limits tap growth"_test = [] {
        AdaptiveLmsFilter<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_taps     = 1U;
        block.step_size  = 0.5;
        block.leak_factor = 0.1;
        block.settingsChanged({}, {});

        // With large step size and leak, taps should stay bounded
        for (int iter = 0; iter < 100; ++iter) {
            const std::array<double, 1> x{1.0}, d{1.0};
            std::array<double, 1>       o{}, e{};
            std::ignore = block.processBulk(std::span{x}, std::span{d}, std::span{o}, std::span{e});
        }
        expect(std::abs(block._taps[0]) < 2.0) << "tap bounded by leakage";
    };
};
