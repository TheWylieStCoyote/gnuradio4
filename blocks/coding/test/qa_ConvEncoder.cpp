#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/coding/ConvEncoder.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"ConvEncoder"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::coding;

    "rate-1/2 output chunk size is 2"_test = [] {
        ConvEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();
        enc.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(enc.output_chunk_size), std::size_t{2}));
    };

    "rate-1/3 output chunk size is 3"_test = [] {
        ConvEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();
        enc.generator_polynomials = std::vector<uint32_t>{0133U, 0171U, 0165U};
        enc.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(enc.output_chunk_size), std::size_t{3}));
    };

    "zero input produces all-zero output"_test = [] {
        ConvEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();
        enc.settingsChanged({}, {});

        const std::vector<uint8_t> in(8, uint8_t{0});
        std::vector<uint8_t>       out(16, uint8_t{1}); // pre-fill with 1 to catch errors
        std::ignore = enc.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});

        for (const auto b : out) {
            expect(eq(b, uint8_t{0}));
        }
    };

    "all-ones input produces non-trivial output"_test = [] {
        ConvEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();
        enc.settingsChanged({}, {});

        const std::vector<uint8_t> in(4, uint8_t{1});
        std::vector<uint8_t>       out(8);
        std::ignore = enc.processBulk(std::span<const uint8_t>{in}, std::span<uint8_t>{out});

        // All outputs must be 0 or 1
        for (const auto b : out) {
            expect(b == uint8_t{0} || b == uint8_t{1});
        }
        // The first input bit shifts into empty register — first pair should be [1, 1]
        // (poly 0133 = 1011011, poly 0171 = 1111001; with K=7 and first bit=1: SR=0000001)
        // 0133 & 0000001 = 0000001 → popcount 1 → 1
        // 0171 & 0000001 = 0000001 → popcount 1 → 1
        expect(eq(out[0], uint8_t{1})) << "first coded bit";
        expect(eq(out[1], uint8_t{1})) << "second coded bit";
    };

    "settingsChanged resets shift register"_test = [] {
        ConvEncoder enc{};
        enc.settings().init();
        std::ignore = enc.settings().applyStagedParameters();
        enc.settingsChanged({}, {});

        // Encode some 1s to load the shift register
        const std::vector<uint8_t> in1(4, uint8_t{1});
        std::vector<uint8_t>       out1(8);
        std::ignore = enc.processBulk(std::span<const uint8_t>{in1}, std::span<uint8_t>{out1});

        // Reset via settingsChanged
        enc.settingsChanged({}, {});

        // Encoding 1 bit should now match fresh state
        const std::vector<uint8_t> in2{uint8_t{1}};
        std::vector<uint8_t>       out2(2);
        std::ignore = enc.processBulk(std::span<const uint8_t>{in2}, std::span<uint8_t>{out2});

        expect(eq(out2[0], uint8_t{1})) << "first bit after reset";
        expect(eq(out2[1], uint8_t{1})) << "second bit after reset";
    };
};
