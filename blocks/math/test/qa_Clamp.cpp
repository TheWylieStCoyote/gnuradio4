#include <boost/ut.hpp>
#include <complex>

#include <gnuradio-4.0/math/Clamp.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Clamp"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    auto makeBlock = []<typename T>(T lo, T hi) {
        Clamp<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.min   = lo;
        block.max   = hi;
        return block;
    };

    "values within range pass through unchanged"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{-2}, T{2});
        expect(approx(static_cast<double>(block.processOne(T{0})), 0.0, 1e-9)) << "zero";
        expect(approx(static_cast<double>(block.processOne(T{1})), 1.0, 1e-9)) << "positive";
        expect(approx(static_cast<double>(block.processOne(T{-1})), -1.0, 1e-9)) << "negative";
    } | kTypes;

    "values above max are clipped to max"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{-1}, T{1});
        expect(approx(static_cast<double>(block.processOne(T{5})), 1.0, 1e-9)) << "above max";
        expect(approx(static_cast<double>(block.processOne(T{100})), 1.0, 1e-9)) << "far above max";
    } | kTypes;

    "values below min are clipped to min"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{-1}, T{1});
        expect(approx(static_cast<double>(block.processOne(T{-5})), -1.0, 1e-9)) << "below min";
        expect(approx(static_cast<double>(block.processOne(T{-100})), -1.0, 1e-9)) << "far below min";
    } | kTypes;

    "boundary values are preserved"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{-3}, T{3});
        expect(approx(static_cast<double>(block.processOne(T{3})), 3.0, 1e-9)) << "at max";
        expect(approx(static_cast<double>(block.processOne(T{-3})), -3.0, 1e-9)) << "at min";
    } | kTypes;

    "complex: real and imaginary parts clipped independently"_test = [] {
        using T  = std::complex<double>;
        using VT = double;
        Clamp<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.min   = VT{-1};
        block.max   = VT{1};

        const T result = block.processOne(T{3.0, -5.0});
        expect(approx(result.real(), 1.0, 1e-9)) << "real clipped to max";
        expect(approx(result.imag(), -1.0, 1e-9)) << "imag clipped to min";
    };

    "complex: in-range samples pass through"_test = [] {
        using T  = std::complex<float>;
        using VT = float;
        Clamp<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.min   = VT{-2};
        block.max   = VT{2};

        const T result = block.processOne(T{1.5f, -0.5f});
        expect(approx(static_cast<double>(result.real()), 1.5, 1e-5)) << "real in range";
        expect(approx(static_cast<double>(result.imag()), -0.5, 1e-5)) << "imag in range";
    };
};
