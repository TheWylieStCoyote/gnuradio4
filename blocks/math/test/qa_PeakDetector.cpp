#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/math/PeakDetector.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"PeakDetector"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    auto makeBlock = []<typename T>(gr::Size_t la, T minH, gr::Size_t minD) {
        PeakDetector<T> block({});
        block.settings().init();
        std::ignore             = block.settings().applyStagedParameters();
        block.look_ahead        = la;
        block.min_peak_height   = minH;
        block.min_peak_distance = minD;
        block.tag_key           = "";  // no tag publishing in these tests
        block.settingsChanged({}, {});
        return block;
    };

    "single isolated peak is detected"_test = [&makeBlock]<typename T> {
        constexpr gr::Size_t kLA = 2U;
        // signal: 0 1 0 0 0 — peak at index 1
        // output delayed by look_ahead samples
        auto block = makeBlock.template operator()<T>(kLA, T{0}, gr::Size_t{1});

        std::vector<T> in{T{0}, T{1}, T{0}, T{0}, T{0}};
        std::vector<T> out(5, T{-1});
        std::ignore = block.processBulk(std::span{in}, std::span{out});

        // Look_ahead = 2: candidate at index 1 is confirmed after seeing in[2] and in[3]
        // Output at i=3 corresponds to candidate in[1], output at i=4 to candidate in[2]
        bool foundPeak = false;
        for (auto v : out) {
            if (v == T{1}) { foundPeak = true; }
        }
        expect(foundPeak) << "peak of amplitude 1 should appear in output";
    } | std::tuple<float, double>{};

    "below-threshold peak is suppressed"_test = [] {
        PeakDetector<float> block({});
        block.settings().init();
        std::ignore             = block.settings().applyStagedParameters();
        block.look_ahead        = 2U;
        block.min_peak_height   = 5.f;
        block.min_peak_distance = 1U;
        block.tag_key           = "";
        block.settingsChanged({}, {});

        std::vector<float> in{0.f, 3.f, 0.f, 0.f, 0.f}; // peak at 3 < threshold 5
        std::vector<float> out(5, -1.f);
        std::ignore = block.processBulk(std::span{in}, std::span{out});

        for (auto v : out) {
            expect(v <= 0.f) << "sub-threshold peak suppressed";
        }
    };

    "settingsChanged resets internal state"_test = [] {
        PeakDetector<float> block({});
        block.settings().init();
        std::ignore             = block.settings().applyStagedParameters();
        block.look_ahead        = 4U;
        block.min_peak_height   = 0.f;
        block.min_peak_distance = 1U;
        block.tag_key           = "";
        block.settingsChanged({}, {});
        expect(eq(block._filled, std::size_t{0}));
        expect(eq(block._buf.size(), std::size_t{5})); // look_ahead + 1
    };
};
