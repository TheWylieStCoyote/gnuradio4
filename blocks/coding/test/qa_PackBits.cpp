#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/PackBits.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"PackBits"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "8-bit pack: 8 bits become one byte MSB-first"_test = [] {
        PackBits block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.bits_per_chunk = 8U;
        block.settingsChanged({}, {});

        // bits: 1 0 1 0 1 1 0 0 → 0xAC
        const std::vector<uint8_t> in{1, 0, 1, 0, 1, 1, 0, 0};
        std::vector<uint8_t>       out(1);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});
        expect(eq(out[0], uint8_t{0xAC})) << "packed byte";
    };

    "4-bit pack: two nibbles in sequence"_test = [] {
        PackBits block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.bits_per_chunk = 4U;
        block.settingsChanged({}, {});

        // 1 0 1 0 → 0x0A, 1 1 0 0 → 0x0C
        const std::vector<uint8_t> in{1, 0, 1, 0,  1, 1, 0, 0};
        std::vector<uint8_t>       out(2);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});
        expect(eq(out[0], uint8_t{0x0A})) << "first nibble";
        expect(eq(out[1], uint8_t{0x0C})) << "second nibble";
    };

    "unpack then pack roundtrips cleanly"_test = [] {
        PackBits block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.bits_per_chunk = 8U;
        block.settingsChanged({}, {});

        const std::vector<uint8_t> in{0, 1, 0, 1, 1, 1, 0, 1}; // 0x5D
        std::vector<uint8_t>       out(1);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});
        expect(eq(out[0], uint8_t{0x5D}));
    };
};
