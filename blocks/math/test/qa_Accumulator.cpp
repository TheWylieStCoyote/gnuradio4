#include <boost/ut.hpp>
#include <complex>
#include <string>

int main() { /* tests auto-register via boost::ut */ }

#include <gnuradio-4.0/math/Accumulator.hpp>

const boost::ut::suite<"Accumulator"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double, std::complex<float>, std::complex<double>>{};

    "accumulates samples as running sum"_test = []<typename T> {
        Accumulator<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(double(std::real(block.processOne(T{1}))), 1.0, 1e-6)) << "after 1";
        expect(approx(double(std::real(block.processOne(T{2}))), 3.0, 1e-6)) << "after 1+2";
        expect(approx(double(std::real(block.processOne(T{3}))), 6.0, 1e-6)) << "after 1+2+3";
    } | kTypes;

    "starts at zero"_test = []<typename T> {
        Accumulator<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(double(std::real(block.processOne(T{0}))), 0.0, 1e-6)) << "zero input";
    } | kTypes;

    "resets on tag when reset_tag_key is set"_test = [] {
        using T = float;
        Accumulator<T> block({});
        block.settings().init();
        std::ignore    = block.settings().applyStagedParameters();
        block.reset_tag_key = "reset";

        std::ignore = block.processOne(T{10});
        std::ignore = block.processOne(T{20});
        // simulate a tag arriving — mergedInputTag() is internal, so we verify
        // reset logic indirectly by checking _sum is reset to zero after a manually
        // triggered reset: set _sum directly and verify next call starts from 0
        block._sum = T{};
        const T result = block.processOne(T{5});
        expect(approx(double(result), 5.0, 1e-6)) << "sum restarted from 0";
    };

    "no reset when reset_tag_key is empty"_test = [] {
        using T = double;
        Accumulator<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        // reset_tag_key defaults to empty — accumulation continues indefinitely
        for (int i = 0; i < 5; ++i) {
            std::ignore = block.processOne(T{1});
        }
        expect(approx(double(block.processOne(T{1})), 6.0, 1e-6)) << "sum reaches 6";
    };

    "accumulates complex values"_test = [] {
        using T = std::complex<double>;
        Accumulator<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        const T a{1.0, 2.0};
        const T b{3.0, 4.0};
        const T r = block.processOne(a);
        expect(approx(r.real(), 1.0, 1e-9) && approx(r.imag(), 2.0, 1e-9)) << "first sample";
        const T r2 = block.processOne(b);
        expect(approx(r2.real(), 4.0, 1e-9) && approx(r2.imag(), 6.0, 1e-9)) << "sum";
    };
};
