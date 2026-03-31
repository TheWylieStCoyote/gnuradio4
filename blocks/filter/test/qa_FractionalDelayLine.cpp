#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/filter/FractionalDelayLine.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"FractionalDelayLine"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    "zero delay passes signal through (within filter warm-up)"_test = []<typename T> {
        FractionalDelayLine<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.delay  = static_cast<typename FractionalDelayLine<T>::value_type>(0);
        block.n_taps = 4U;
        block.settingsChanged({}, {});

        // tap[0] = 1, rest = 0 for d=0 → passes current sample unchanged
        std::vector<T> out;
        for (int i = 0; i < 8; ++i) {
            out.push_back(block.processOne(static_cast<T>(i + 1)));
        }
        // after warm-up (n_taps samples), output should track input
        expect(approx(static_cast<double>(out[4]), 5.0, 0.1)) << "d=0 passes through";
    } | std::tuple<float, double>{};

    "integer delay shifts signal by correct number of samples"_test = [] {
        // delay = 2: output[n] should equal input[n-2]
        FractionalDelayLine<float> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.delay  = 2.f;
        block.n_taps = 4U;
        block.settingsChanged({}, {});

        std::vector<float> in{0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f};
        std::vector<float> out;
        for (float x : in) {
            out.push_back(block.processOne(x));
        }
        // impulse at index 2 should appear at index 4 in the output (delay=2)
        expect(out[4] > 0.9f) << "impulse at delay=2 appears at out[4]";
    };

    "settingsChanged reinitialises taps"_test = [] {
        FractionalDelayLine<double> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.delay  = 1.5;
        block.n_taps = 4U;
        block.settingsChanged({}, {});
        expect(eq(block._taps.size(), std::size_t{4}));
        expect(block._taps[0] != 0.0);
    };
};
