#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/StreamDemux.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"StreamDemux"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "resizes outputs and sets chunk sizes on settingsChanged"_test = []<typename T> {
        StreamDemux<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_outputs  = 3U;
        block.chunk_size = 2U;
        block.settingsChanged({}, {});

        expect(eq(block.outputs.size(), std::size_t{3}));
        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{6}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{2}));
    } | std::tuple<float, double>{};

    "default two outputs one sample each"_test = [] {
        StreamDemux<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_outputs  = 2U;
        block.chunk_size = 1U;
        block.settingsChanged({}, {});

        expect(eq(block.outputs.size(), std::size_t{2}));
        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{2}));
    };
};
