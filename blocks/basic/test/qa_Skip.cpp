#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/Skip.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Skip"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "first n_samples produce zero, rest pass through"_test = []<typename T> {
        Skip<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_samples = 3U;
        block.settingsChanged({}, {});

        std::vector<T> out;
        for (int i = 0; i < 6; ++i) out.push_back(block.processOne(static_cast<T>(i + 1)));

        // First 3 suppressed
        for (int i = 0; i < 3; ++i) {
            expect(eq(out[static_cast<std::size_t>(i)], T{})) << "suppressed at i=" << i;
        }
        // Next 3 pass through
        for (int i = 3; i < 6; ++i) {
            expect(eq(out[static_cast<std::size_t>(i)], static_cast<T>(i + 1))) << "passed at i=" << i;
        }
    } | std::tuple<float, double>{};

    "n_samples=0 passes all samples unchanged"_test = [] {
        Skip<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_samples = 0U;
        block.settingsChanged({}, {});

        for (int i = 0; i < 8; ++i) {
            expect(eq(block.processOne(static_cast<float>(i)), static_cast<float>(i)));
        }
    };

    "settingsChanged resets skip counter"_test = [] {
        Skip<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_samples = 2U;
        block.settingsChanged({}, {});

        std::ignore = block.processOne(1.0);
        std::ignore = block.processOne(1.0); // fully skipped
        block.settingsChanged({}, {});
        // after reset, first sample should be zero again
        expect(eq(block.processOne(99.0), 0.0)) << "skip counter reset";
    };
};
