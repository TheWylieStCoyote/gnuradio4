#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/filter/SteadyStateKalman.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"SteadyStateKalman"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    auto makeBlock = []<typename T>(T a, T c, T q, T r) {
        SteadyStateKalman<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.A = a;
        block.C = c;
        block.Q = q;
        block.R = r;
        block.settingsChanged({}, {});
        return block;
    };

    "constant signal is tracked with low error (A=1, C=1)"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{1}, T{1}, static_cast<T>(0.01), T{1});

        // Drive with noisy constant signal (using deterministic values for reproducibility)
        T lastOut{};
        for (int i = 0; i < 200; ++i) {
            const T noise = static_cast<T>(0.1 * ((i % 3) - 1)); // ±0.1
            lastOut = block.processOne(T{1} + noise);
        }
        expect(approx(static_cast<double>(lastOut), 1.0, 0.3)) << "tracks constant signal";
    } | std::tuple<float, double>{};

    "gain is non-zero and finite after settingsChanged"_test = [&makeBlock] {
        auto block = makeBlock.template operator()<double>(1.0, 1.0, 0.1, 1.0);
        expect(block._gain > 0.0 && block._gain < 1.0) << "gain in (0,1) for stable system";
    };

    "filter with moderate Q/R converges near signal mean"_test = [&makeBlock] {
        // Q=0.1, R=1 → moderate gain; converges to signal mean after many steps
        auto block = makeBlock.template operator()<double>(1.0, 1.0, 0.1, 1.0);

        double lastOut = 0.0;
        for (int i = 0; i < 500; ++i) {
            const double noise = (i % 2 == 0) ? 0.3 : -0.3;
            lastOut = block.processOne(1.0 + noise);
        }
        expect(approx(lastOut, 1.0, 0.5)) << "converges near signal mean";
    };

    "settingsChanged resets state to zero"_test = [&makeBlock] {
        auto block = makeBlock.template operator()<float>(1.f, 1.f, 0.1f, 1.f);
        for (int i = 0; i < 20; ++i) std::ignore = block.processOne(static_cast<float>(i));
        block.settingsChanged({}, {});
        expect(eq(block._xHat, 0.f)) << "state reset after settingsChanged";
    };
};
