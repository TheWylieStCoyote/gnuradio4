#include <boost/ut.hpp>

#include <gnuradio-4.0/math/Threshold.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Threshold"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    auto makeBlock = []<typename T>(T thresh, T lo, T hi) {
        Threshold<T> block({});
        block.settings().init();
        std::ignore       = block.settings().applyStagedParameters();
        block.threshold   = thresh;
        block.low_value   = lo;
        block.high_value  = hi;
        return block;
    };

    "input above threshold emits high_value"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{0}, T{0}, T{1});
        expect(eq(block.processOne(static_cast<T>(0.1)), T{1}));
        expect(eq(block.processOne(T{100}), T{1}));
    } | std::tuple<float, double>{};

    "input at threshold emits low_value"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(static_cast<T>(0.5), T{0}, T{1});
        expect(eq(block.processOne(static_cast<T>(0.5)), T{0}));
    } | std::tuple<float, double>{};

    "input below threshold emits low_value"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{0}, T{0}, T{1});
        expect(eq(block.processOne(T{-1}), T{0}));
        expect(eq(block.processOne(T{0}),  T{0}));
    } | std::tuple<float, double>{};

    "custom high and low values are used"_test = [] {
        Threshold<float> block({});
        block.settings().init();
        std::ignore    = block.settings().applyStagedParameters();
        block.threshold  = 5.0f;
        block.low_value  = -2.0f;
        block.high_value = 7.0f;
        expect(eq(block.processOne(6.0f), 7.0f));
        expect(eq(block.processOne(4.9f), -2.0f));
    };
};
