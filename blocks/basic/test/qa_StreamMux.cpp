#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/StreamMux.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"StreamMux"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "interleaves two inputs one sample at a time"_test = []<typename T> {
        StreamMux<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.n_inputs   = 2U;
        block.chunk_size = 1U;
        block.settingsChanged({}, {});

        expect(eq(block.inputs.size(), std::size_t{2}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{2}));
    } | std::tuple<float, double>{};

    "resizes inputs on settingsChanged"_test = [] {
        StreamMux<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        block.n_inputs   = 4U;
        block.chunk_size = 3U;
        block.settingsChanged({}, {});

        expect(eq(block.inputs.size(), std::size_t{4}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{12}));
        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{3}));
    };
};
