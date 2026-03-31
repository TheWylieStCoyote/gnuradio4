#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/TagGate.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"TagGate"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "gate closed by default — output is zero"_test = []<typename T> {
        TagGate<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.initial_open = false;
        block.settingsChanged({}, {});

        const T out = block.processOne(static_cast<T>(42));
        expect(eq(out, T{})) << "closed gate zeros the sample";
    } | std::tuple<float, double>{};

    "gate open by default — sample passes through"_test = [] {
        TagGate<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.initial_open = true;
        block.settingsChanged({}, {});

        const float out = block.processOne(3.14f);
        expect(eq(out, 3.14f));
    };

    "settingsChanged resets gate to initial_open"_test = [] {
        TagGate<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.initial_open = true;
        block.settingsChanged({}, {});
        expect(block._open) << "gate open after settingsChanged with initial_open=true";

        block.initial_open = false;
        block.settingsChanged({}, {});
        expect(!block._open) << "gate closed after settingsChanged with initial_open=false";
    };
};
