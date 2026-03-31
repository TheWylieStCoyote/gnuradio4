#include <boost/ut.hpp>

#include <gnuradio-4.0/coding/GrayCodeDecoder.hpp>
#include <gnuradio-4.0/coding/GrayCodeEncoder.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"GrayCodeDecoder"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "decode(encode(n)) == n for all 8-bit values"_test = [] {
        GrayCodeEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();

        GrayCodeDecoder dec{};
        dec.settings().init();
        std::ignore = dec.settings().applyStagedParameters();

        for (int n = 0; n < 256; ++n) {
            const uint8_t b      = static_cast<uint8_t>(n);
            const uint8_t gray   = enc.processOne(b);
            const uint8_t binary = dec.processOne(gray);
            expect(eq(binary, b)) << "round-trip for n=" << n;
        }
    };
};
