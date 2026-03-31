#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>

#include <gnuradio-4.0/math/MovingRms.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"MovingRms"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    auto makeBlock = []<typename T>() {
        MovingRms<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        return block;
    };

    "constant signal RMS equals signal magnitude"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>();
        block.length = gr::Size_t{4U};
        block.settingsChanged({}, {{"length", gr::Size_t{4U}}});

        // After warm-up, RMS of constant 3.0 = 3.0
        for (int i = 0; i < 3; ++i) {
            std::ignore = block.processOne(T{3});
        }
        const double rms = static_cast<double>(block.processOne(T{3}));
        expect(approx(rms, 3.0, 1e-5)) << "constant signal RMS";
    } | kTypes;

    "RMS of unit impulse decays as window fills"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>();
        block.length = gr::Size_t{4U};
        block.settingsChanged({}, {{"length", gr::Size_t{4U}}});

        // impulse at n=0 then zeros
        const double r0 = static_cast<double>(block.processOne(T{1})); // sqrt(1/1) = 1
        const double r1 = static_cast<double>(block.processOne(T{0})); // sqrt(1/2)
        const double r2 = static_cast<double>(block.processOne(T{0})); // sqrt(1/3)
        const double r3 = static_cast<double>(block.processOne(T{0})); // sqrt(1/4) = 0.5
        const double r4 = static_cast<double>(block.processOne(T{0})); // impulse leaves window → 0

        expect(approx(r0, 1.0, 1e-5)) << "n=0";
        expect(approx(r1, std::sqrt(0.5), 1e-5)) << "n=1";
        expect(approx(r2, std::sqrt(1.0 / 3.0), 1e-5)) << "n=2";
        expect(approx(r3, 0.5, 1e-5)) << "n=3 (full window)";
        expect(approx(r4, 0.0, 1e-5)) << "n=4 (impulse gone)";
    } | kTypes;

    "complex input: RMS of unit complex samples is 1"_test = [] {
        using T       = std::complex<double>;
        using VT      = double;
        MovingRms<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.length = gr::Size_t{4U};
        block.settingsChanged({}, {{"length", gr::Size_t{4U}}});

        // |1+0j|² = 1, |0+1j|² = 1 — RMS should settle at 1
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(T{1.0, 0.0});
        }
        const VT rms = block.processOne(T{0.0, 1.0});
        expect(approx(static_cast<double>(rms), 1.0, 1e-9)) << "complex unit samples";
    };

    "output is always real"_test = [] {
        using T  = std::complex<float>;
        using VT = float;
        MovingRms<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        static_assert(std::is_same_v<decltype(block.processOne(T{})), VT>,
                      "output type must be real value_type");
        expect(true);
    };

    "length=1 returns instantaneous magnitude"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>();
        block.length = gr::Size_t{1U};
        block.settingsChanged({}, {{"length", gr::Size_t{1U}}});

        expect(approx(static_cast<double>(block.processOne(T{5})), 5.0, 1e-5)) << "length=1";
        expect(approx(static_cast<double>(block.processOne(T{3})), 3.0, 1e-5)) << "length=1 next";
    } | kTypes;

    "settings reset clears state"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>();
        block.length = gr::Size_t{4U};
        block.settingsChanged({}, {{"length", gr::Size_t{4U}}});

        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(T{10});
        }
        // reset to length=1
        block.length = gr::Size_t{1U};
        block.settingsChanged({}, {{"length", gr::Size_t{1U}}});

        const double rms = static_cast<double>(block.processOne(T{2}));
        expect(approx(rms, 2.0, 1e-5)) << "after reset";
    } | kTypes;
};
