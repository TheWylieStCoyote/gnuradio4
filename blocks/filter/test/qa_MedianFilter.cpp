#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/filter/MedianFilter.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"MedianFilter"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    "output is zero until window is full"_test = []<typename T> {
        MedianFilter<T> f{};
        f.settings().init();
        std::ignore = f.settings().applyStagedParameters();
        f.window_size = 3U;
        f.settingsChanged({}, {});

        expect(eq(f.processOne(T{1}), T{})) << "sample 1";
        expect(eq(f.processOne(T{2}), T{})) << "sample 2";
        // window is now full after 3rd sample
        expect(neq(f.processOne(T{3}), T{})) << "sample 3";
    } | std::tuple<float, double>{};

    "median of odd window"_test = [] {
        MedianFilter<float> f{};
        f.settings().init();
        std::ignore = f.settings().applyStagedParameters();
        f.window_size = 5U;
        f.settingsChanged({}, {});

        // Feed [1, 3, 2, 5, 4] — after 5 samples window = [1,3,2,5,4], sorted = [1,2,3,4,5], median = 3
        for (float v : {1.f, 3.f, 2.f, 5.f, 4.f}) {
            std::ignore = f.processOne(v);
        }
        std::ignore = f.processOne(0.f); // slide window: [3,2,5,4,0], sorted = [0,2,3,4,5], median = 3
        const float result = f.processOne(10.f); // slide: [2,5,4,0,10], sorted = [0,2,4,5,10], median = 4
        expect(eq(result, 4.f));
    };

    "median with window size 1 is identity"_test = [] {
        MedianFilter<float> f{};
        f.settings().init();
        std::ignore = f.settings().applyStagedParameters();
        f.window_size = 1U;
        f.settingsChanged({}, {});

        expect(eq(f.processOne(7.f), 7.f));
        expect(eq(f.processOne(3.f), 3.f));
        expect(eq(f.processOne(9.f), 9.f));
    };

    "constant signal returns same value"_test = [] {
        MedianFilter<float> f{};
        f.settings().init();
        std::ignore = f.settings().applyStagedParameters();
        f.window_size = 4U;
        f.settingsChanged({}, {});

        float last = 0.f;
        for (int i = 0; i < 8; ++i) {
            last = f.processOne(5.f);
        }
        expect(eq(last, 5.f));
    };

    "settingsChanged resets history"_test = [] {
        MedianFilter<float> f{};
        f.settings().init();
        std::ignore = f.settings().applyStagedParameters();
        f.window_size = 3U;
        f.settingsChanged({}, {});

        std::ignore = f.processOne(1.f);
        std::ignore = f.processOne(2.f);
        std::ignore = f.processOne(3.f);

        f.settingsChanged({}, {});
        // After reset, first two outputs should be zero again
        expect(eq(f.processOne(10.f), 0.f));
        expect(eq(f.processOne(20.f), 0.f));
    };
};
