#include <boost/ut.hpp>
#include <complex>
#include <numbers>

#include <gnuradio-4.0/math/QuadratureDemod.hpp>

const boost::ut::suite<"QuadratureDemod"> quadratureDemodTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<std::complex<float>, std::complex<double>>{};

    "zero input gives zero output"_test = []<typename T> {
        QuadratureDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(double(block.processOne(T{0, 0})), 0.0, 1e-6)) << "zero input";
    } | kTypes;

    "constant phasor gives zero frequency"_test = []<typename T> {
        using V = typename T::value_type;
        QuadratureDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const T phasor{V{1}, V{0}};
        std::ignore = block.processOne(phasor); // prime _prev
        expect(approx(double(block.processOne(phasor)), 0.0, 1e-6)) << "constant phasor → zero frequency";
    } | kTypes;

    "quarter-cycle rotation gives π/2 per sample"_test = []<typename T> {
        using V                     = typename T::value_type;
        constexpr double kHalfPi    = std::numbers::pi / 2.0;
        QuadratureDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(T{V{1}, V{0}}); // prime _prev
        expect(approx(double(block.processOne(T{V{0}, V{1}})), kHalfPi, 1e-5)) << "quarter-cycle → π/2";
    } | kTypes;

    "gain scales output linearly"_test = [] {
        using T                  = std::complex<float>;
        constexpr double kHalfPi = std::numbers::pi / 2.0;

        QuadratureDemod<T> block({{"gain", 2.f}});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(T{1.f, 0.f});
        expect(approx(double(block.processOne(T{0.f, 1.f})), 2.0 * kHalfPi, 1e-5)) << "gain=2 doubles output";
    };

    "full cycle phase rotation sums to 2π"_test = [] {
        using T                = std::complex<float>;
        constexpr double kTwoPi = 2.0 * std::numbers::pi;

        QuadratureDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // Five phasors: 0°→90°→180°→270°→360°(=0°) — four π/2 steps total
        const std::array<T, 5> phasors{T{1.f, 0.f}, T{0.f, 1.f}, T{-1.f, 0.f}, T{0.f, -1.f}, T{1.f, 0.f}};
        std::ignore = block.processOne(phasors[0]); // prime
        double sum  = 0.0;
        for (std::size_t i = 1; i < phasors.size(); ++i) {
            sum += double(block.processOne(phasors[i]));
        }
        expect(approx(sum, kTwoPi, 1e-5)) << "four steps sum to 2π";
    };

    "negative rotation gives negative frequency"_test = [] {
        using T                  = std::complex<float>;
        constexpr double kHalfPi = std::numbers::pi / 2.0;

        QuadratureDemod<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(T{0.f, 1.f}); // prime _prev = j
        expect(approx(double(block.processOne(T{1.f, 0.f})), -kHalfPi, 1e-5)) << "clockwise rotation → negative";
    };
};

int main() { /* tests auto-register via boost::ut */ }
