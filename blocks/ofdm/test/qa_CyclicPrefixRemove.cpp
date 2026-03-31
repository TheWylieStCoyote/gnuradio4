#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/ofdm/CyclicPrefixAdd.hpp>
#include <gnuradio-4.0/ofdm/CyclicPrefixRemove.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"CyclicPrefixRemove"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::ofdm;

    "output is fft_size samples"_test = []<typename T> {
        CyclicPrefixRemove<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size  = 8U;
        block.cp_length = 2U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{10}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{8}));
    } | std::tuple<float, double>{};

    "cyclic prefix samples are discarded"_test = [] {
        CyclicPrefixRemove<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size  = 4U;
        block.cp_length = 2U;
        block.settingsChanged({}, {});

        // Input: [3, 4, 1, 2, 3, 4] where first 2 are the CP
        const std::vector<float> in{3.f, 4.f, 1.f, 2.f, 3.f, 4.f};
        std::vector<float>       out(4);
        std::ignore = block.processBulk(std::span<const float>{in}, std::span<float>{out});

        expect(eq(out[0], 1.f)) << "symbol sample 0";
        expect(eq(out[1], 2.f)) << "symbol sample 1";
        expect(eq(out[2], 3.f)) << "symbol sample 2";
        expect(eq(out[3], 4.f)) << "symbol sample 3";
    };

    "add and remove are inverse operations"_test = [] {
        CyclicPrefixAdd<float>    addBlock{};
        CyclicPrefixRemove<float> removeBlock{};
        addBlock.settings().init();
        std::ignore = addBlock.settings().applyStagedParameters();
        removeBlock.settings().init();
        std::ignore = removeBlock.settings().applyStagedParameters();
        addBlock.fft_size    = 8U;
        addBlock.cp_length   = 4U;
        removeBlock.fft_size  = 8U;
        removeBlock.cp_length = 4U;
        addBlock.settingsChanged({}, {});
        removeBlock.settingsChanged({}, {});

        const std::vector<float> symbol{1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
        std::vector<float>       withCp(12);
        std::ignore = addBlock.processBulk(std::span<const float>{symbol}, std::span<float>{withCp});

        std::vector<float> recovered(8);
        std::ignore = removeBlock.processBulk(std::span<const float>{withCp}, std::span<float>{recovered});

        for (std::size_t i = 0; i < 8; ++i) {
            expect(eq(recovered[i], symbol[i])) << "roundtrip at i=" << i;
        }
    };
};
