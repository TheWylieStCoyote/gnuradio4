#include <boost/ut.hpp>
#include <complex>
#include <numbers>

#include <gnuradio-4.0/math/Conjugate.hpp>
#include <gnuradio-4.0/math/Differentiator.hpp>
#include <gnuradio-4.0/math/PhaseUnwrap.hpp>

// ─── PhaseUnwrap ────────────────────────────────────────────────────────────

const boost::ut::suite<"PhaseUnwrap"> phaseUnwrapTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    "monotonically rising phase is preserved"_test = []<typename T> {
        PhaseUnwrap<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // 0 → π/4 → π/2 → 3π/4 → π: no wrap, output equals input
        constexpr T kQuarterPi = std::numbers::pi_v<T> / T{4};
        T           expected   = T{};
        for (int i = 0; i < 4; ++i) {
            expected += kQuarterPi;
            expect(approx(double(block.processOne(expected)), double(expected), 1e-6)) << "no wrap yet";
        }
    } | kTypes;

    "2π jump is unwrapped"_test = [] {
        PhaseUnwrap<float> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        constexpr float kPi    = std::numbers::pi_v<float>;
        constexpr float kTwoPi = 2.f * kPi;

        std::ignore = block.processOne(kPi - 0.1f); // just below π
        // jump to just above -π (equivalent to wrapping past π)
        const double result = double(block.processOne(-kPi + 0.1f));
        // unwrapped: should continue monotonically (≈ π + 0.1)
        expect(approx(result, double(kPi + 0.1f), 0.01)) << "positive jump unwrapped";
        std::ignore = kTwoPi; // suppress unused warning
    };

    "negative jump is unwrapped"_test = [] {
        PhaseUnwrap<float> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        constexpr float kPi = std::numbers::pi_v<float>;

        std::ignore = block.processOne(-kPi + 0.1f); // just above -π
        // jump to just below π (wrapping in the negative direction)
        const double result = double(block.processOne(kPi - 0.1f));
        expect(approx(result, double(-kPi - 0.1f), 0.01)) << "negative jump unwrapped";
    };

    "multiple wraps accumulate correctly"_test = [] {
        PhaseUnwrap<double> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        constexpr double kPi    = std::numbers::pi;
        constexpr double kStep  = kPi * 0.9; // just under π per sample

        double expected = 0.0;
        for (int i = 0; i < 10; ++i) {
            expected += kStep;
            const double wrapped  = std::fmod(expected + kPi, 2.0 * kPi) - kPi;
            const double unwrapped = double(block.processOne(wrapped));
            expect(approx(unwrapped, expected, 1e-9)) << "accumulated unwrap";
        }
    };
};

// ─── Conjugate ──────────────────────────────────────────────────────────────

const boost::ut::suite<"Conjugate"> conjugateTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<std::complex<float>, std::complex<double>>{};

    "conjugate negates imaginary part"_test = []<typename T> {
        using V = typename T::value_type;
        Conjugate<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const T input{V{3}, V{4}};
        const T result = block.processOne(input);
        expect(approx(double(result.real()), 3.0, 1e-9)) << "real unchanged";
        expect(approx(double(result.imag()), -4.0, 1e-9)) << "imag negated";
    } | kTypes;

    "conjugate of real-valued complex is unchanged"_test = []<typename T> {
        using V = typename T::value_type;
        Conjugate<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const T input{V{5}, V{0}};
        const T result = block.processOne(input);
        expect(approx(double(result.real()), 5.0, 1e-9)) << "real part";
        expect(approx(double(result.imag()), 0.0, 1e-9)) << "imag still zero";
    } | kTypes;

    "double conjugate is identity"_test = []<typename T> {
        using V = typename T::value_type;
        Conjugate<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const T input{V{2}, V{-7}};
        expect(approx(double(block.processOne(block.processOne(input)).real()), 2.0, 1e-9));
        expect(approx(double(block.processOne(block.processOne(input)).imag()), -7.0, 1e-9));
    } | kTypes;
};

// ─── Differentiator ─────────────────────────────────────────────────────────

const boost::ut::suite<"Differentiator"> differentiatorTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double, std::complex<float>, std::complex<double>>{};

    "first output equals input (prev = 0)"_test = []<typename T> {
        Differentiator<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(double(std::real(block.processOne(T{5}))), 5.0, 1e-9)) << "first sample";
    } | kTypes;

    "constant input gives zero after first sample"_test = []<typename T> {
        Differentiator<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(T{3});
        expect(approx(double(std::real(block.processOne(T{3}))), 0.0, 1e-9)) << "constant → zero diff";
        expect(approx(double(std::real(block.processOne(T{3}))), 0.0, 1e-9)) << "constant → zero diff";
    } | kTypes;

    "ramp input gives constant difference"_test = [] {
        Differentiator<float> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(0.f);
        for (int i = 1; i <= 5; ++i) {
            expect(approx(double(block.processOne(static_cast<float>(i))), 1.0, 1e-6)) << "ramp diff = 1";
        }
    };

    "complex input differentiates real and imag independently"_test = [] {
        using C = std::complex<float>;
        Differentiator<C> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        std::ignore = block.processOne(C{1.f, 2.f});
        const C diff = block.processOne(C{4.f, 6.f});
        expect(approx(double(diff.real()), 3.0, 1e-6)) << "real diff";
        expect(approx(double(diff.imag()), 4.0, 1e-6)) << "imag diff";
    };
};

int main() { /* tests auto-register via boost::ut */ }
