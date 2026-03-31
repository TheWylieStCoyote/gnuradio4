#include <boost/ut.hpp>
#include <complex>
#include <cstddef>
#include <numeric>
#include <vector>

#include <gnuradio-4.0/filter/DCBlocker.hpp>

template<typename T>
[[nodiscard]] double toDouble(T v) {
    return static_cast<double>(std::real(v));
}

const boost::ut::suite<"DCBlocker"> dcBlockerTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    constexpr auto kTypes = std::tuple<float, double, std::complex<float>, std::complex<double>>{};

    "pure DC signal is attenuated to zero in steady state"_test = []<typename T> {
        constexpr gr::Size_t kLength = 4U;
        DCBlocker<T>         block({{"length", kLength}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime window
        for (std::size_t i = 0; i < kLength; ++i) {
            std::ignore = block.processOne(T{3});
        }
        // window fully filled with the same constant → mean = 3, output = 3 - 3 = 0
        const double result = toDouble(block.processOne(T{3}));
        expect(approx(result, 0.0, 1e-9)) << "steady-state DC rejection";
    } | kTypes;

    "zero-mean signal passes through unchanged in steady state"_test = [] {
        // after the window is full with alternating ±1, the mean stays near 0
        // and the output tracks the input
        constexpr gr::Size_t kLength = 4U;
        DCBlocker<float>     block({{"length", kLength}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime with two full cycles of [1, -1, 1, -1]
        for (int i = 0; i < static_cast<int>(kLength); ++i) {
            std::ignore = block.processOne(i % 2 == 0 ? 1.f : -1.f);
        }
        // window: [1, -1, 1, -1] → mean = 0 → output = input
        const float next = 1.f;
        expect(approx(toDouble(block.processOne(next)), 1.0, 1e-6)) << "AC passes through";
    };

    "warm-up phase subtracts partial-window mean"_test = [] {
        // With a constant input of 6 and window length 4:
        //   after sample 1: mean = 6/1 = 6  → output = 6 - 6 = 0
        //   after sample 2: mean = 12/2 = 6 → output = 6 - 6 = 0
        DCBlocker<double> block({{"length", gr::Size_t{4U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(toDouble(block.processOne(6.0)), 0.0, 1e-9)) << "after 1 sample";
        expect(approx(toDouble(block.processOne(6.0)), 0.0, 1e-9)) << "after 2 samples";
    };

    "DC offset is removed when mixed with AC"_test = [] {
        // Input: DC = 5, AC = sin-like square wave ±1
        // After the window fills the DC estimate converges to 5 → output ≈ ±1
        constexpr gr::Size_t kLength = 4U;
        DCBlocker<float>     block({{"length", kLength}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const std::array<float, 4U> prime{6.f, 4.f, 6.f, 4.f}; // DC=5, AC=±1
        for (float v : prime) {
            std::ignore = block.processOne(v);
        }
        // window: [4,6,4,6] → mean = 5 → output = next - 5
        expect(approx(toDouble(block.processOne(6.f)), 1.0, 1e-5)) << "AC component preserved";
        expect(approx(toDouble(block.processOne(4.f)), -1.0, 1e-5)) << "AC component preserved";
    };

    "complex input blocks DC on real and imag independently"_test = [] {
        using C = std::complex<float>;
        DCBlocker<C> block({{"length", gr::Size_t{2U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime: [2+4i, 2+4i] → mean = 2+4i
        std::ignore = block.processOne(C{2.f, 4.f});
        std::ignore = block.processOne(C{2.f, 4.f});
        // next input = 2+4i, mean = 2+4i → output = 0+0i
        const C result = block.processOne(C{2.f, 4.f});
        expect(approx(double(result.real()), 0.0, 1e-5)) << "real DC removed";
        expect(approx(double(result.imag()), 0.0, 1e-5)) << "imag DC removed";
    };

    "length=1 always outputs zero"_test = [] {
        // mean of the single-sample window equals the input → output = 0 for all inputs
        DCBlocker<float> block({{"length", gr::Size_t{1U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        for (const float v : {0.f, 1.f, -3.f, 100.f}) {
            expect(approx(toDouble(block.processOne(v)), 0.0, 1e-9)) << "length=1 always zero";
        }
    };

    "settings change resets filter state"_test = [] {
        DCBlocker<float> block({{"length", gr::Size_t{4U}}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // prime with constant DC; history is full of 10
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(10.f);
        }

        // reset to length=2 — all state must clear
        block.length = gr::Size_t{2U};
        block.settingsChanged({}, {{"length", gr::Size_t{2U}}});

        // first sample: mean = 5/1 = 5 → output = 5 - 5 = 0
        expect(approx(toDouble(block.processOne(5.f)), 0.0, 1e-9)) << "first sample after reset";
        // second sample: mean = (5+3)/2 = 4 → output = 3 - 4 = -1
        expect(approx(toDouble(block.processOne(3.f)), -1.0, 1e-9)) << "second sample after reset";
    };
};

int main() { /* tests auto-register via boost::ut */ }
