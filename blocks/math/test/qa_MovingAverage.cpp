#include <boost/ut.hpp>
#include <complex>
#include <cstddef>
#include <vector>

#include <gnuradio-4.0/math/MovingAverage.hpp>

// Helper: cast any real or complex sample's real part to double for comparison.
template<typename T>
[[nodiscard]] double toDouble(T v) {
    return static_cast<double>(std::real(v));
}

const boost::ut::suite<"MovingAverage"> movingAverageTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double, std::complex<float>, std::complex<double>>{};

    "steady state equals exact mean over window"_test = []<typename T> {
        constexpr gr::Size_t kLength = 4U;
        MovingAverage<T>     block({{"length", kLength}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime window with the same value
        for (std::size_t i = 0; i < kLength; ++i) {
            std::ignore = block.processOne(T{2});
        }
        // window: [2, 2, 2, 2] → mean = 2
        const double result = toDouble(block.processOne(T{2}));
        expect(approx(result, 2.0, 1e-9)) << "steady-state mean";
    } | kTypes;

    "warm-up phase uses only available samples"_test = [] {
        MovingAverage<float> block({{"length", gr::Size_t{4U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(toDouble(block.processOne(4.f)), 4.0, 1e-9)) << "after 1 sample";
        expect(approx(toDouble(block.processOne(2.f)), 3.0, 1e-9)) << "after 2 samples: (4+2)/2";
        expect(approx(toDouble(block.processOne(0.f)), 2.0, 1e-9)) << "after 3 samples: (4+2+0)/3";
        expect(approx(toDouble(block.processOne(2.f)), 2.0, 1e-9)) << "after 4 samples: (4+2+0+2)/4";
    };

    "oldest sample is evicted once window is full"_test = [] {
        MovingAverage<double> block({{"length", gr::Size_t{3U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime window with zeros
        std::ignore = block.processOne(0.0);
        std::ignore = block.processOne(0.0);
        std::ignore = block.processOne(0.0);

        // spike enters window [10, 0, 0] → mean = 10/3
        expect(approx(toDouble(block.processOne(10.0)), 10.0 / 3.0, 1e-9)) << "spike enters";
        // window [0, 10, 0] → mean = 10/3
        expect(approx(toDouble(block.processOne(0.0)), 10.0 / 3.0, 1e-9)) << "one zero enters";
        // window [0, 0, 10] → mean = 10/3
        expect(approx(toDouble(block.processOne(0.0)), 10.0 / 3.0, 1e-9)) << "two zeros enter";
        // spike evicted [0, 0, 0] → mean = 0
        expect(approx(toDouble(block.processOne(0.0)), 0.0, 1e-9)) << "spike evicted";
    };

    "complex input averages real and imag independently"_test = [] {
        using C = std::complex<float>;
        MovingAverage<C> block({{"length", gr::Size_t{2U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(C{1.f, 3.f});
        const C result = block.processOne(C{3.f, 1.f});
        expect(approx(double(result.real()), 2.0, 1e-6)) << "real part: (1+3)/2";
        expect(approx(double(result.imag()), 2.0, 1e-6)) << "imag part: (3+1)/2";
    };

    "length=1 is a pass-through"_test = [] {
        MovingAverage<float> block({{"length", gr::Size_t{1U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        for (const float v : {1.f, 5.f, -3.f, 0.f}) {
            expect(approx(toDouble(block.processOne(v)), double(v), 1e-9)) << "length=1 pass-through";
        }
    };

    "settings change resets filter state"_test = [] {
        MovingAverage<float> block({{"length", gr::Size_t{4U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime with non-zero history
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(100.f);
        }

        // change length → framework updates the field then invokes settingsChanged
        block.length = gr::Size_t{2U};
        block.settingsChanged({}, {{"length", gr::Size_t{2U}}});

        expect(approx(toDouble(block.processOne(4.f)), 4.0, 1e-9)) << "first sample after reset";
        expect(approx(toDouble(block.processOne(2.f)), 3.0, 1e-9)) << "second sample after reset";
    };
};

int main() { /* tests auto-register via boost::ut */ }
