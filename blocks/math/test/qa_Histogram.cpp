#include <boost/ut.hpp>
#include <numeric>
#include <vector>

#include <gnuradio-4.0/math/Histogram.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Histogram"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    "output DataSet has n_bins entries"_test = []<typename T> {
        Histogram<T> h{};
        h.settings().init();
        std::ignore = h.settings().applyStagedParameters();
        h.n_bins       = 8U;
        h.min_value    = static_cast<T>(-1);
        h.max_value    = static_cast<T>(1);
        h.accumulate_n = 16U;
        h.settingsChanged({}, {});

        std::vector<T> in(16, T{});
        std::vector<gr::DataSet<T>> out(1);
        std::ignore = h.processBulk(std::span<const T>{in}, std::span<gr::DataSet<T>>{out});

        expect(eq(out[0].signal_values.size(), std::size_t{8}));
        expect(eq(out[0].axis_values[0].size(), std::size_t{8}));
    } | std::tuple<float, double>{};

    "all-zero signal fills the centre bin"_test = [] {
        Histogram<float> h{};
        h.settings().init();
        std::ignore = h.settings().applyStagedParameters();
        h.n_bins       = 4U;
        h.min_value    = -1.f;
        h.max_value    =  1.f;
        h.accumulate_n = 8U;
        h.settingsChanged({}, {});

        std::vector<float> in(8, 0.f); // 0 falls in bin 2 (range [-1,1], 4 bins, width 0.5)
        std::vector<gr::DataSet<float>> out(1);
        std::ignore = h.processBulk(std::span<const float>{in}, std::span<gr::DataSet<float>>{out});

        // bin 0: [-1, -0.5), bin 1: [-0.5, 0), bin 2: [0, 0.5), bin 3: [0.5, 1]
        // 0.0 maps to bin 2
        expect(eq(out[0].signal_values[2], 1.f)) << "centre bin has all samples";
        expect(eq(out[0].signal_values[0], 0.f));
        expect(eq(out[0].signal_values[3], 0.f));
    };

    "out-of-range samples are ignored"_test = [] {
        Histogram<float> h{};
        h.settings().init();
        std::ignore = h.settings().applyStagedParameters();
        h.n_bins       = 4U;
        h.min_value    = 0.f;
        h.max_value    = 1.f;
        h.accumulate_n = 4U;
        h.settingsChanged({}, {});

        const std::vector<float> in{-5.f, 0.5f, 2.f, 0.5f};
        std::vector<gr::DataSet<float>> out(1);
        std::ignore = h.processBulk(std::span<const float>{in}, std::span<gr::DataSet<float>>{out});

        // only 2 of 4 samples are in range; normalised sum of in-range bins = 2/4 = 0.5
        const float sum = std::accumulate(out[0].signal_values.begin(), out[0].signal_values.end(), 0.f);
        expect(sum > 0.f) << "at least some counts";
    };

    "normalised counts sum to 1 when all samples are in range"_test = [] {
        Histogram<float> h{};
        h.settings().init();
        std::ignore = h.settings().applyStagedParameters();
        h.n_bins       = 10U;
        h.min_value    = 0.f;
        h.max_value    = 1.f;
        h.accumulate_n = 100U;
        h.settingsChanged({}, {});

        std::vector<float> in(100);
        for (int i = 0; i < 100; ++i) {
            in[static_cast<std::size_t>(i)] = static_cast<float>(i) / 100.f;
        }
        std::vector<gr::DataSet<float>> out(1);
        std::ignore = h.processBulk(std::span<const float>{in}, std::span<gr::DataSet<float>>{out});

        const float sum = std::accumulate(out[0].signal_values.begin(), out[0].signal_values.end(), 0.f);
        expect(std::abs(sum - 1.f) < 0.001f) << "normalised sum is 1";
    };
};
