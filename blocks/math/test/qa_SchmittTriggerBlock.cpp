#include <boost/ut.hpp>

#include <gnuradio-4.0/math/SchmittTrigger.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"SchmittTrigger"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    auto makeBlock = []<typename T>() {
        SchmittTrigger<T> block({});
        block.settings().init();
        std::ignore           = block.settings().applyStagedParameters();
        block.upper_threshold = static_cast<T>(0.7);
        block.lower_threshold = static_cast<T>(0.3);
        block.high_value      = T{1};
        block.low_value       = T{0};
        return block;
    };

    "output starts low and rises only above upper_threshold"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>();
        expect(eq(block.processOne(static_cast<T>(0.5)),  T{0})) << "below upper, stays low";
        expect(eq(block.processOne(static_cast<T>(0.71)), T{1})) << "crosses upper, goes high";
        expect(eq(block.processOne(static_cast<T>(0.5)),  T{1})) << "in hysteresis band, stays high";
    } | std::tuple<float, double>{};

    "output falls only below lower_threshold"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>();
        std::ignore = block.processOne(T{1});   // go high
        expect(eq(block.processOne(static_cast<T>(0.31)), T{1})) << "above lower, stays high";
        expect(eq(block.processOne(static_cast<T>(0.29)), T{0})) << "crosses lower, goes low";
        expect(eq(block.processOne(static_cast<T>(0.5)),  T{0})) << "in band, stays low";
    } | std::tuple<float, double>{};

    "no toggling on noise within hysteresis band"_test = [] {
        SchmittTrigger<float> block({});
        block.settings().init();
        std::ignore           = block.settings().applyStagedParameters();
        block.upper_threshold = 0.8f;
        block.lower_threshold = 0.2f;
        block.high_value      = 1.f;
        block.low_value       = 0.f;

        // inject signal that oscillates in the middle — output must remain 0
        for (int i = 0; i < 100; ++i) {
            const float x = (i % 2 == 0) ? 0.5f : 0.6f;
            expect(eq(block.processOne(x), 0.f));
        }
    };
};
