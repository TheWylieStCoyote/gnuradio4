#include <boost/ut.hpp>

#include <gnuradio-4.0/coding/GrayCodeEncoder.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"GrayCodeEncoder"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "known Gray code values are correct"_test = [] {
        GrayCodeEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        // Standard 4-bit Gray code table
        const std::vector<std::pair<uint8_t, uint8_t>> table{
            {0, 0}, {1, 1}, {2, 3}, {3, 2}, {4, 6}, {5, 7}, {6, 5}, {7, 4},
        };
        for (auto [bin, gray] : table) {
            expect(eq(enc.processOne(bin), gray)) << "bin=" << static_cast<int>(bin);
        }
    };

    "adjacent outputs differ in exactly one bit"_test = [] {
        GrayCodeEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        for (uint8_t i = 0; i < 255; ++i) {
            const uint8_t g0 = enc.processOne(i);
            const uint8_t g1 = enc.processOne(static_cast<uint8_t>(i + 1));
            const int diff = __builtin_popcount(g0 ^ g1);
            expect(eq(diff, 1)) << "adjacent codes differ in one bit at i=" << static_cast<int>(i);
        }
    };
};
