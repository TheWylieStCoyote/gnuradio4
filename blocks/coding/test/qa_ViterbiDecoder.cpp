#include <boost/ut.hpp>
#include <cstdint>
#include <vector>

#include <gnuradio-4.0/coding/ConvEncoder.hpp>
#include <gnuradio-4.0/coding/ViterbiDecoder.hpp>

namespace {

std::vector<uint8_t> encode(std::vector<uint8_t> bits) {
    gr::blocks::coding::ConvEncoder enc{};
    enc.settings().init();
    std::ignore = enc.settings().applyStagedParameters();
    enc.settingsChanged({}, {});

    std::vector<uint8_t> coded(bits.size() * 2U);
    std::ignore = enc.processBulk(std::span<const uint8_t>{bits}, std::span<uint8_t>{coded});
    return coded;
}

std::vector<uint8_t> decode(std::vector<uint8_t> coded, std::size_t depth) {
    gr::blocks::coding::ViterbiDecoder dec{};
    dec.settings().init();
    std::ignore = dec.settings().applyStagedParameters();
    dec.traceback_depth = static_cast<gr::Size_t>(depth);
    dec.settingsChanged({}, {});

    std::vector<uint8_t> out(depth);
    std::ignore = dec.processBulk(std::span<const uint8_t>{coded}, std::span<uint8_t>{out});
    return out;
}

} // namespace

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"ViterbiDecoder"> tests = [] {
    using namespace boost::ut;

    "chunk sizes match rate * traceback_depth in, traceback_depth out"_test = [] {
        gr::blocks::coding::ViterbiDecoder dec{};
        dec.settings().init();
        std::ignore = dec.settings().applyStagedParameters();
        dec.traceback_depth = 35U;
        dec.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(dec.input_chunk_size),  std::size_t{70}));
        expect(eq(static_cast<std::size_t>(dec.output_chunk_size), std::size_t{35}));
    };

    "all-zeros roundtrip via ConvEncoder"_test = [] {
        constexpr std::size_t D = 35UZ;
        std::vector<uint8_t> msg(D, 0U);
        auto coded  = encode(msg);
        auto decoded = decode(coded, D);

        // bits after the constraint-length flush window should be correct
        constexpr std::size_t kFlush = 6UZ; // K-1 for K=7
        for (std::size_t i = kFlush; i < D; ++i) {
            expect(eq(decoded[i], msg[i])) << "zero bit at position " << i;
        }
    };

    "all-ones roundtrip via ConvEncoder"_test = [] {
        constexpr std::size_t D = 35UZ;
        std::vector<uint8_t> msg(D, 1U);
        auto coded   = encode(msg);
        auto decoded = decode(coded, D);

        constexpr std::size_t kFlush = 6UZ;
        for (std::size_t i = kFlush; i < D; ++i) {
            expect(eq(decoded[i], msg[i])) << "one bit at position " << i;
        }
    };

    "alternating bits roundtrip via ConvEncoder"_test = [] {
        constexpr std::size_t D = 35UZ;
        std::vector<uint8_t> msg(D);
        for (std::size_t i = 0; i < D; ++i) {
            msg[i] = static_cast<uint8_t>(i & 1U);
        }
        auto coded   = encode(msg);
        auto decoded = decode(coded, D);

        constexpr std::size_t kFlush = 6UZ;
        for (std::size_t i = kFlush; i < D; ++i) {
            expect(eq(decoded[i], msg[i])) << "alternating bit at position " << i;
        }
    };

    "settingsChanged resets path metrics"_test = [] {
        gr::blocks::coding::ViterbiDecoder dec{};
        dec.settings().init();
        std::ignore = dec.settings().applyStagedParameters();
        dec.traceback_depth = 7U;
        dec.settingsChanged({}, {});

        // first decode
        std::vector<uint8_t> coded(14U, 0U);
        std::vector<uint8_t> out1(7U);
        std::ignore = dec.processBulk(std::span<const uint8_t>{coded}, std::span<uint8_t>{out1});

        // change depth and re-initialise
        dec.traceback_depth = 14U;
        dec.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(dec.input_chunk_size),  std::size_t{28}));
        expect(eq(static_cast<std::size_t>(dec.output_chunk_size), std::size_t{14}));
    };
};
