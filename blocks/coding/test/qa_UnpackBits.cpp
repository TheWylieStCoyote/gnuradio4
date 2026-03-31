#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/UnpackBits.hpp>
#include <gnuradio-4.0/coding/PackBits.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"UnpackBits"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "8-bit unpack: byte 0xAC gives bits MSB-first"_test = [] {
        UnpackBits block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.bits_per_chunk = 8U;
        block.settingsChanged({}, {});

        // 0xAC = 1010 1100
        const std::vector<uint8_t> in{0xAC};
        std::vector<uint8_t>       out(8);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});

        const std::vector<uint8_t> expected{1, 0, 1, 0, 1, 1, 0, 0};
        for (std::size_t i = 0; i < 8; ++i) {
            expect(eq(out[i], expected[i])) << "bit " << i;
        }
    };

    "pack(unpack(byte)) == byte for all bytes"_test = [] {
        UnpackBits unpackBlock{};
        unpackBlock.settings().init();
        std::ignore = unpackBlock.settings().applyStagedParameters();
        unpackBlock.bits_per_chunk = 8U;
        unpackBlock.settingsChanged({}, {});

        PackBits packBlock{};
        packBlock.settings().init();
        std::ignore = packBlock.settings().applyStagedParameters();
        packBlock.bits_per_chunk = 8U;
        packBlock.settingsChanged({}, {});

        for (int b = 0; b < 256; ++b) {
            const std::vector<uint8_t> in{static_cast<uint8_t>(b)};
            std::vector<uint8_t>       bits(8), packed(1);
            std::ignore = unpackBlock.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{bits});
            std::ignore = packBlock.processBulk(std::span<const uint8_t>{bits}, std::span<uint8_t>{packed});
            expect(eq(packed[0], static_cast<uint8_t>(b))) << "round-trip for byte=" << b;
        }
    };
};
