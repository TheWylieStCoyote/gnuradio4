#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/Scrambler.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Scrambler"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    auto makeBlock = [](uint32_t mask, uint32_t seed, gr::Size_t len) {
        Scrambler block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.mask = mask;
        block.seed = seed;
        block.len  = len;
        block.settingsChanged({}, {});
        return block;
    };

    "scramble then descramble recovers original bits"_test = [&makeBlock] {
        auto enc = makeBlock(0xB8U, 0xFFU, 8U);
        auto dec = makeBlock(0xB8U, 0xFFU, 8U);

        const std::vector<uint8_t> bits{1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1};
        for (std::size_t i = 0; i < bits.size(); ++i) {
            const uint8_t scrambled   = enc.processOne(bits[i]);
            const uint8_t descrambled = dec.processOne(scrambled);
            expect(eq(descrambled, bits[i])) << "round-trip at bit " << i;
        }
    };

    "scrambled output differs from input (non-trivial)"_test = [&makeBlock] {
        auto block = makeBlock(0xB8U, 0xFFU, 8U);
        const std::vector<uint8_t> zeros(16, 0);
        std::vector<uint8_t> out;
        for (uint8_t b : zeros) out.push_back(block.processOne(b));

        // Scrambled zero stream should not be all zeros (LFSR is non-trivial)
        const bool allZero = std::all_of(out.begin(), out.end(), [](uint8_t v) { return v == 0; });
        expect(!allZero) << "scrambler whitens zero stream";
    };

    "settingsChanged resets LFSR to seed"_test = [&makeBlock] {
        auto block = makeBlock(0xB8U, 0xFFU, 8U);

        std::vector<uint8_t> run1, run2;
        for (int i = 0; i < 8; ++i) run1.push_back(block.processOne(0));

        block.settingsChanged({}, {});
        for (int i = 0; i < 8; ++i) run2.push_back(block.processOne(0));

        expect(run1 == run2) << "same output after reset";
    };

    "output bits are 0 or 1"_test = [&makeBlock] {
        auto block = makeBlock(0xB8U, 0x01U, 8U);
        for (int i = 0; i < 32; ++i) {
            const uint8_t out = block.processOne(static_cast<uint8_t>(i & 1));
            expect(out <= 1U) << "output is binary";
        }
    };
};
