#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/CrcCompute.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"CrcCompute"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    auto makeBlock = [](gr::Size_t pktLen) {
        CrcCompute block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.packet_len = pktLen;
        block.settingsChanged({}, {});
        return block;
    };

    "output length is packet_len + 1"_test = [&makeBlock] {
        const gr::Size_t L = 4U;
        auto block = makeBlock(L);

        const std::vector<uint8_t> in{0xAA, 0xBB, 0xCC, 0xDD};
        std::vector<uint8_t>       out(5);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});

        // First L bytes are unchanged
        for (std::size_t i = 0; i < 4; ++i) {
            expect(eq(out[i], in[i])) << "data byte " << i << " unchanged";
        }
        // Last byte is CRC
        const uint8_t expectedCrc = CrcCompute::computeCrc(std::span<const uint8_t>{in});
        expect(eq(out[4], expectedCrc)) << "CRC byte correct";
    };

    "CRC of all-zeros data is deterministic"_test = [&makeBlock] {
        auto block = makeBlock(4U);

        std::vector<uint8_t> in(4, 0), out(5);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});
        const uint8_t crc1 = out[4];

        // Same input → same CRC
        std::vector<uint8_t> out2(5);
        std::ignore = block.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out2});
        expect(eq(out2[4], crc1)) << "CRC is deterministic";
    };

    "CRC changes when data changes"_test = [&makeBlock] {
        auto block = makeBlock(4U);

        std::vector<uint8_t> in1{0x01, 0x02, 0x03, 0x04};
        std::vector<uint8_t> in2{0x01, 0x02, 0x03, 0x05}; // last byte differs
        std::vector<uint8_t> out1(5), out2(5);

        std::ignore = block.processBulk(std::span<const uint8_t>{in1}, std::span<uint8_t>{out1});
        std::ignore = block.processBulk(std::span<const uint8_t>{in2}, std::span<uint8_t>{out2});
        expect(out1[4] != out2[4]) << "different data produces different CRC";
    };
};
