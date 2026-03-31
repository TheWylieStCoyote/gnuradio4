#include <boost/ut.hpp>
#include <complex>
#include <vector>

#include <gnuradio-4.0/demod/SymbolSync.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"SymbolSync"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::demod;

    "settingsChanged initialises omega to sps"_test = []<typename T> {
        SymbolSync<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.sps = static_cast<T>(8);
        block.settingsChanged({}, {});

        expect(approx(static_cast<double>(block._omega), 8.0, 0.01)) << "omega starts at sps";
        expect(block._kp > T{}) << "kp > 0";
        expect(block._ki > T{}) << "ki > 0";
    } | std::tuple<float, double>{};

    "PI gains scale with sps"_test = [] {
        SymbolSync<double> block1{};
        block1.settings().init();
        std::ignore = block1.settings().applyStagedParameters();
        block1.sps = 4.0;
        block1.settingsChanged({}, {});

        SymbolSync<double> block2{};
        block2.settings().init();
        std::ignore = block2.settings().applyStagedParameters();
        block2.sps = 8.0;
        block2.settingsChanged({}, {});

        // Larger sps → smaller gains (per-sample gains inversely scale with sps)
        expect(block1._kp > block2._kp) << "kp decreases with larger sps";
    };
};
