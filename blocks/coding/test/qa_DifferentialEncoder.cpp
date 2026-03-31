#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/DifferentialEncoder.hpp>
#include <gnuradio-4.0/coding/DifferentialDecoder.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"DifferentialEncoder"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "encoding and decoding are inverses"_test = [] {
        DifferentialEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        DifferentialDecoder dec{};
        dec.settings().init();
        std::ignore = dec.settings().applyStagedParameters();

        const std::vector<uint8_t> bits{0, 1, 1, 0, 1, 0, 0, 1};
        std::vector<uint8_t>       encoded, decoded;

        for (uint8_t b : bits) encoded.push_back(enc.processOne(b));
        for (uint8_t b : encoded) decoded.push_back(dec.processOne(b));

        for (std::size_t i = 0; i < bits.size(); ++i) {
            expect(eq(decoded[i], bits[i])) << "round-trip bit " << i;
        }
    };

    "encoding a zero sequence keeps previous bit"_test = [] {
        DifferentialEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        // All zeros: out stays 0 XOR 0 = 0
        for (int i = 0; i < 8; ++i) {
            expect(eq(enc.processOne(0), uint8_t{0})) << "zero stays zero";
        }
    };

    "settingsChanged resets state"_test = [] {
        DifferentialEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        // encode a 1 to set _prev = 1
        std::ignore = enc.processOne(1);
        enc.settingsChanged({}, {});
        // after reset _prev=0, encoding 0 should give 0
        expect(eq(enc.processOne(0), uint8_t{0})) << "state reset";
    };

    "only LSB is used"_test = [] {
        DifferentialEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        // byte 0xFF has LSB=1, same as encoding 1
        const uint8_t out = enc.processOne(0xFF);
        expect(out <= 1U) << "output is 0 or 1";
    };
};
