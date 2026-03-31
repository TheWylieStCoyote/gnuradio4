#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/DifferentialDecoder.hpp>
#include <gnuradio-4.0/coding/DifferentialEncoder.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"DifferentialDecoder"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "decoding encoded sequence recovers original bits"_test = [] {
        DifferentialEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        DifferentialDecoder dec{};
        dec.settings().init();
        std::ignore = dec.settings().applyStagedParameters();

        const std::vector<uint8_t> bits{1, 0, 1, 1, 0, 0, 1, 0};
        for (std::size_t i = 0; i < bits.size(); ++i) {
            const uint8_t enc_out = enc.processOne(bits[i]);
            const uint8_t dec_out = dec.processOne(enc_out);
            expect(eq(dec_out, bits[i])) << "round-trip at bit " << i;
        }
    };

    "settingsChanged resets state"_test = [] {
        DifferentialDecoder dec{};
        dec.settings().init();
        std::ignore = dec.settings().applyStagedParameters();

        std::ignore = dec.processOne(1);  // set _prev = 1
        dec.settingsChanged({}, {});
        // after reset _prev=0; decoding 1 gives 1 XOR 0 = 1
        expect(eq(dec.processOne(1), uint8_t{1})) << "state reset";
    };
};
