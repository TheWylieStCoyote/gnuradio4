#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/ofdm/CyclicPrefixAdd.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"CyclicPrefixAdd"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::ofdm;

    "output is fft_size + cp_length"_test = []<typename T> {
        CyclicPrefixAdd<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size  = 8U;
        block.cp_length = 2U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{8}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{10}));
    } | std::tuple<float, double>{};

    "last cp samples are prepended"_test = [] {
        CyclicPrefixAdd<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size  = 4U;
        block.cp_length = 2U;
        block.settingsChanged({}, {});

        const std::vector<float> in{1.f, 2.f, 3.f, 4.f};
        std::vector<float>       out(6);
        std::ignore = block.processBulk(std::span<const float>{in}, std::span<float>{out});

        // Expected: [3, 4, 1, 2, 3, 4] (last 2 samples prepended)
        expect(eq(out[0], 3.f)) << "cyclic prefix sample 0";
        expect(eq(out[1], 4.f)) << "cyclic prefix sample 1";
        expect(eq(out[2], 1.f)) << "symbol sample 0";
        expect(eq(out[3], 2.f)) << "symbol sample 1";
        expect(eq(out[4], 3.f)) << "symbol sample 2";
        expect(eq(out[5], 4.f)) << "symbol sample 3";
    };
};
