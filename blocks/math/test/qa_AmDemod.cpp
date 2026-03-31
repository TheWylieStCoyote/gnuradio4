#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>

#include <gnuradio-4.0/math/AmDemod.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"AmDemod"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<std::complex<float>, std::complex<double>>{};

    "magnitude of real-only input equals real part"_test = []<typename T> {
        AmDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const auto r = block.processOne(T{3, 0});
        expect(approx(static_cast<double>(r), 3.0, 1e-6)) << "real-only";
    } | kTypes;

    "magnitude of imaginary-only input equals imag part"_test = []<typename T> {
        AmDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const auto r = block.processOne(T{0, 4});
        expect(approx(static_cast<double>(r), 4.0, 1e-6)) << "imag-only";
    } | kTypes;

    "Pythagorean triple: |3+4j| = 5"_test = []<typename T> {
        AmDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const auto r = block.processOne(T{3, 4});
        expect(approx(static_cast<double>(r), 5.0, 1e-5)) << "3-4-5 triangle";
    } | kTypes;

    "output is always non-negative"_test = []<typename T> {
        AmDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(ge(static_cast<double>(block.processOne(T{-3, -4})), 0.0)) << "negative input";
        expect(ge(static_cast<double>(block.processOne(T{0, 0})), 0.0)) << "zero";
    } | kTypes;

    "unit phasor has magnitude 1 at all angles"_test = []<typename T> {
        using VT = typename T::value_type;
        AmDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        constexpr double kPi     = std::numbers::pi;
        const double     angles[] = {0.0, kPi / 4, kPi / 2, kPi, 3 * kPi / 2};
        for (const double angle : angles) {
            const T input{static_cast<VT>(std::cos(angle)), static_cast<VT>(std::sin(angle))};
            expect(approx(static_cast<double>(block.processOne(input)), 1.0, 1e-5))
                << "angle=" << angle;
        }
    } | kTypes;

    "output type is real value_type"_test = [] {
        using T  = std::complex<float>;
        using VT = float;
        AmDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        static_assert(std::is_same_v<decltype(block.processOne(T{})), VT>,
                      "output must be real value_type");
        expect(true);
    };
};
