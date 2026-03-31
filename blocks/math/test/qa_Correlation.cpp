#include <boost/ut.hpp>
#include <complex>
#include <vector>

#include <gnuradio-4.0/math/Correlation.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Correlation"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    auto makeBlock = []<typename T>(gr::Size_t w) {
        Correlation<T> block({});
        block.settings().init();
        std::ignore      = block.settings().applyStagedParameters();
        block.window_size = w;
        block.settingsChanged({}, {});
        return block;
    };

    "identical signals give power estimate"_test = [&makeBlock]<typename T> {
        constexpr gr::Size_t kW = 4U;
        auto block = makeBlock.template operator()<T>(kW);

        std::vector<T> sig{T{2}, T{2}, T{2}, T{2}};
        std::vector<T> ref = sig;
        std::vector<T> out(4, T{});

        std::ignore = block.processBulk(std::span{sig}, std::span{ref}, std::span{out});

        // after warm-up: out[W-1] = mean(|2|²) = 4
        expect(approx(static_cast<double>(out[kW - 1]), 4.0, 1e-4)) << "fully-filled window";
    } | std::tuple<float, double>{};

    "orthogonal signals give zero correlation"_test = [] {
        constexpr gr::Size_t kW = 8U;
        Correlation<float> block({});
        block.settings().init();
        std::ignore      = block.settings().applyStagedParameters();
        block.window_size = kW;
        block.settingsChanged({}, {});

        // alternating +1/-1 vs constant +1 → sum over window ≈ 0
        std::vector<float> sig(kW), ref(kW, 1.f);
        for (std::size_t i = 0; i < kW; ++i) sig[i] = (i % 2 == 0) ? 1.f : -1.f;
        std::vector<float> out(kW);
        std::ignore = block.processBulk(std::span{sig}, std::span{ref}, std::span{out});

        expect(approx(static_cast<double>(out[kW - 1]), 0.0, 1e-4)) << "orthogonal → zero";
    };

    "complex: settingsChanged resets state"_test = [] {
        using CT = std::complex<float>;
        Correlation<CT> block({});
        block.settings().init();
        std::ignore      = block.settings().applyStagedParameters();
        block.window_size = 8U;
        block.settingsChanged({}, {});
        expect(eq(block._filledCount, std::size_t{0}));
    };
};
