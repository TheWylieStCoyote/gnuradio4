#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/StreamToVector.hpp>
#include <gnuradio-4.0/basic/VectorToStream.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"StreamToVector"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "chunk sizes are set correctly"_test = []<typename T> {
        StreamToVector<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.vlen = 8U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size),  std::size_t{8}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{1}));
    } | std::tuple<float, double>{};

    "samples are packed into DataSet signal_values"_test = [] {
        StreamToVector<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.vlen = 4U;
        block.settingsChanged({}, {});

        const std::vector<float> in{1.f, 2.f, 3.f, 4.f};
        std::vector<gr::DataSet<float>> out(1);
        std::ignore = block.processBulk(std::span<const float>{in}, std::span<gr::DataSet<float>>{out});

        expect(eq(out[0].signal_values.size(), std::size_t{4}));
        expect(eq(out[0].signal_values[0], 1.f));
        expect(eq(out[0].signal_values[3], 4.f));
        expect(eq(out[0].signal_names[0], std::string{"samples"}));
    };

    "roundtrip: StreamToVector then VectorToStream recovers original signal"_test = [] {
        StreamToVector<float> toVec{};
        VectorToStream<float> toStream{};
        for (auto* b : std::initializer_list<StreamToVector<float>*>{&toVec}) {
            b->settings().init();
            std::ignore = b->settings().applyStagedParameters();
        }
        toVec.vlen    = 6U;
        toStream.vlen = 6U;
        toVec.settingsChanged({}, {});
        toStream.settings().init();
        std::ignore = toStream.settings().applyStagedParameters();
        toStream.settingsChanged({}, {});

        const std::vector<float>        in{1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
        std::vector<gr::DataSet<float>> mid(1);
        std::ignore = toVec.processBulk(std::span<const float>{in}, std::span<gr::DataSet<float>>{mid});

        std::vector<float> out(6);
        std::ignore = toStream.processBulk(std::span<const gr::DataSet<float>>{mid}, std::span<float>{out});

        for (std::size_t i = 0; i < 6; ++i) {
            expect(eq(out[i], in[i])) << "sample " << i;
        }
    };
};

const boost::ut::suite<"VectorToStream"> tests2 = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "chunk sizes are set correctly"_test = []<typename T> {
        VectorToStream<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.vlen = 8U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size),  std::size_t{1}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{8}));
    } | std::tuple<float, double>{};

    "short DataSet is zero-padded to vlen"_test = [] {
        VectorToStream<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.vlen = 4U;
        block.settingsChanged({}, {});

        gr::DataSet<float> ds{};
        ds.signal_values = {1.f, 2.f};

        std::vector<gr::DataSet<float>> in{ds};
        std::vector<float>              out(4, -1.f);
        std::ignore = block.processBulk(std::span<const gr::DataSet<float>>{in}, std::span<float>{out});

        expect(eq(out[0], 1.f));
        expect(eq(out[1], 2.f));
        expect(eq(out[2], 0.f)); // zero-padded
        expect(eq(out[3], 0.f));
    };
};
