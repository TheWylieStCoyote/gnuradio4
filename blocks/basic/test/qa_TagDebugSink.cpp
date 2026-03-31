#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/Tag.hpp>
#include <gnuradio-4.0/basic/TagDebugSink.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"TagDebugSink"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "passes samples through unchanged"_test = []<typename T> {
        TagDebugSink<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.verbose = false;

        const T out = block.processOne(static_cast<T>(42));
        expect(eq(out, static_cast<T>(42)));
    } | std::tuple<float, double>{};

    "starts with no received tags"_test = [] {
        TagDebugSink<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(block._receivedTags.empty()) << "no tags on init";
    };

    "settingsChanged clears received tags"_test = [] {
        TagDebugSink<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        block._receivedTags.push_back(gr::Tag{0, {}});
        expect(eq(block._receivedTags.size(), std::size_t{1}));

        block.settingsChanged({}, {});
        expect(block._receivedTags.empty()) << "tags cleared after settingsChanged";
    };
};
