#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/Head.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Head"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "processOne passes samples unchanged"_test = []<typename T> {
        Head<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_samples = 4U;
        block.settingsChanged({}, {});

        for (int i = 0; i < 3; ++i) {
            const T x   = static_cast<T>(i + 1);
            const T out = block.processOne(x);
            expect(eq(out, x)) << "sample " << i << " passed through";
        }
    } | std::tuple<float, double>{};

    "settingsChanged resets sample counter"_test = [] {
        Head<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_samples = 2U;
        block.settingsChanged({}, {});

        std::ignore = block.processOne(1.f);
        std::ignore = block.processOne(1.f); // reaches n_samples, requestStop() called
        block.settingsChanged({}, {});        // reset
        expect(eq(block._count, std::size_t{0})) << "counter reset";
    };
};
