#include <boost/ut.hpp>
#include <cmath>
#include <complex>

#include <gnuradio-4.0/math/Limiter.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Limiter"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    auto makeBlock = []<typename T>(T lim) {
        Limiter<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.limit  = lim;
        return block;
    };

    "values within limit pass through unchanged"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{2});
        expect(approx(static_cast<double>(block.processOne(T{1})), 1.0, 1e-9)) << "positive in range";
        expect(approx(static_cast<double>(block.processOne(T{-1})), -1.0, 1e-9)) << "negative in range";
        expect(approx(static_cast<double>(block.processOne(T{0})), 0.0, 1e-9)) << "zero";
    } | kTypes;

    "positive values above limit are clipped"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{1});
        expect(approx(static_cast<double>(block.processOne(T{5})), 1.0, 1e-9)) << "positive clip";
    } | kTypes;

    "negative values below -limit are clipped"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{1});
        expect(approx(static_cast<double>(block.processOne(T{-5})), -1.0, 1e-9)) << "negative clip";
    } | kTypes;

    "boundary values are preserved exactly"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{3});
        expect(approx(static_cast<double>(block.processOne(T{3})), 3.0, 1e-9)) << "at +limit";
        expect(approx(static_cast<double>(block.processOne(T{-3})), -3.0, 1e-9)) << "at -limit";
    } | kTypes;

    "complex: magnitude is clamped, phase is preserved"_test = [] {
        using T  = std::complex<double>;
        using VT = double;
        Limiter<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.limit  = VT{1};

        // magnitude 5, angle 0 → magnitude 1, angle 0
        const T r1 = block.processOne(T{5, 0});
        expect(approx(std::abs(r1), 1.0, 1e-9)) << "magnitude clipped to 1";
        expect(approx(std::arg(r1), 0.0, 1e-9)) << "phase preserved (0)";

        // |3+4j| = 5, angle = atan2(4,3)
        const T input{3.0, 4.0};
        const T r2 = block.processOne(input);
        expect(approx(std::abs(r2), 1.0, 1e-9)) << "3+4j clipped to unit";
        expect(approx(std::arg(r2), std::arg(input), 1e-9)) << "phase preserved";
    };

    "complex: in-range values pass through unchanged"_test = [] {
        using T  = std::complex<float>;
        using VT = float;
        Limiter<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.limit  = VT{2};

        const T input{1.0f, 0.5f};
        const T result = block.processOne(input);
        expect(approx(static_cast<double>(result.real()), 1.0, 1e-5)) << "real unchanged";
        expect(approx(static_cast<double>(result.imag()), 0.5, 1e-5)) << "imag unchanged";
    };
};
