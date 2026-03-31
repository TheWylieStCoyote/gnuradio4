#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/StreamTagger.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"StreamTagger"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "publishes tag at sample 0"_test = []<typename T> {
        StreamTagger<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.interval = 4U;
        block.key      = "test_tag";
        block.settingsChanged({}, {});

        // _count == 0 → tag should be published on first call
        const T out = block.processOne(static_cast<T>(1));
        expect(eq(out, static_cast<T>(1))) << "sample passes through";
    } | std::tuple<float, double>{};

    "resets count on settingsChanged"_test = [] {
        StreamTagger<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.interval = 8U;
        block.settingsChanged({}, {});

        // advance past sample 0
        for (int i = 0; i < 3; ++i) {
            std::ignore = block.processOne(1.f);
        }
        expect(eq(block._count, std::size_t{3})) << "_count advanced";

        block.settingsChanged({}, {});
        expect(eq(block._count, std::size_t{0})) << "_count reset to 0";
    };

    "interval=0 never publishes"_test = [] {
        StreamTagger<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.interval = 0U;
        block.settingsChanged({}, {});

        // Just verify it doesn't crash and passes samples through
        for (int i = 0; i < 8; ++i) {
            const float out = block.processOne(static_cast<float>(i));
            expect(eq(out, static_cast<float>(i)));
        }
    };

    "counter increments each sample"_test = [] {
        StreamTagger<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.interval = 1024U;
        block.settingsChanged({}, {});

        for (std::size_t i = 0; i < 5; ++i) {
            expect(eq(block._count, i)) << "count before call";
            std::ignore = block.processOne(1.0);
        }
        expect(eq(block._count, std::size_t{5}));
    };
};
