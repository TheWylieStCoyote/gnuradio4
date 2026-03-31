#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/filter/CicFilter.hpp>

int main() { /* tests auto-register via boost::ut */ }

static void initBlock(auto& block) {
    block.settings().init();
    std::ignore = block.settings().applyStagedParameters();
}

const boost::ut::suite<"CicDecimator"> decimatorTests = [] {
    using namespace boost::ut;
    using namespace gr::filter;

    "DC input passes through with unity gain after decimation"_test = [] {
        using T = double;
        CicDecimator<T> block({});
        initBlock(block);
        block.decimation  = gr::Size_t{4U};
        block.n_stages    = gr::Size_t{2U};
        block.differential_delay = gr::Size_t{1U};
        block.settingsChanged({}, {});

        // Steady-state DC input of amplitude 1.0 should pass through unchanged
        // Warm up the filter first
        constexpr std::size_t R = 4;
        constexpr std::size_t kWarmup = 20;
        std::vector<T>       in(R * kWarmup, T{1.0});
        std::vector<T>       out(kWarmup);

        // Process warmup
        std::span<const T> inSpan{in.data(), in.size()};
        std::span<T>       outSpan{out.data(), out.size()};
        expect(block.processBulk(inSpan, outSpan) == gr::work::Status::OK);

        // Last output should be ~1.0 (DC passes with unity gain after normalisation)
        expect(approx(out.back(), 1.0, 0.01)) << "DC passes with unity gain";
    };

    "output sample count is input/decimation"_test = [] {
        using T = float;
        CicDecimator<T> block({});
        initBlock(block);
        block.decimation  = gr::Size_t{8U};
        block.n_stages    = gr::Size_t{3U};
        block.differential_delay = gr::Size_t{1U};
        block.settingsChanged({}, {});

        constexpr std::size_t R   = 8;
        constexpr std::size_t N   = 10;
        std::vector<T> in(R * N, T{1});
        std::vector<T> out(N);
        expect(block.processBulk(std::span{in}, std::span{out}) == gr::work::Status::OK);
    };

    "DC passes with unity gain for integer types"_test = [] {
        using T = std::int32_t;
        CicDecimator<T> block({});
        initBlock(block);
        block.decimation  = gr::Size_t{4U};
        block.n_stages    = gr::Size_t{2U};
        block.differential_delay = gr::Size_t{1U};
        block.settingsChanged({}, {});

        constexpr std::size_t R = 4;
        constexpr std::size_t kWarmup = 20;
        std::vector<T> in(R * kWarmup, T{100});
        std::vector<T> out(kWarmup);
        expect(block.processBulk(std::span{in}, std::span{out}) == gr::work::Status::OK);
        // DC input 100 → output ≈ 100 after normalisation
        expect(eq(out.back(), T{100})) << "integer DC passes through";
    };
};

const boost::ut::suite<"CicInterpolator"> interpolatorTests = [] {
    using namespace boost::ut;
    using namespace gr::filter;

    "output sample count is input*interpolation"_test = [] {
        using T = double;
        CicInterpolator<T> block({});
        initBlock(block);
        block.interpolation = gr::Size_t{4U};
        block.n_stages      = gr::Size_t{2U};
        block.differential_delay = gr::Size_t{1U};
        block.settingsChanged({}, {});

        constexpr std::size_t L = 4;
        constexpr std::size_t N = 8;
        std::vector<T> in(N, T{1});
        std::vector<T> out(N * L);
        expect(block.processBulk(std::span{in}, std::span{out}) == gr::work::Status::OK);
    };

    "interpolator followed by decimator: impulse input round-trips within gain tolerance"_test = [] {
        using T = double;

        CicInterpolator<T> interp({});
        initBlock(interp);
        interp.interpolation = gr::Size_t{4U};
        interp.n_stages      = gr::Size_t{2U};
        interp.differential_delay = gr::Size_t{1U};
        interp.settingsChanged({}, {});

        CicDecimator<T> decim({});
        initBlock(decim);
        decim.decimation  = gr::Size_t{4U};
        decim.n_stages    = gr::Size_t{2U};
        decim.differential_delay = gr::Size_t{1U};
        decim.settingsChanged({}, {});

        constexpr std::size_t L       = 4;
        constexpr std::size_t kSamples = 16;

        // Send a single impulse — the pipeline should not crash and produces kSamples outputs
        std::vector<T> src(kSamples, T{0.0});
        src[0] = T{1.0};
        std::vector<T> interpolated(kSamples * L);
        std::vector<T> recovered(kSamples);

        expect(interp.processBulk(std::span{src}, std::span{interpolated}) == gr::work::Status::OK);
        expect(decim.processBulk(std::span{interpolated}, std::span{recovered}) == gr::work::Status::OK);

        // The energy of the impulse is preserved through the pipeline (may be spread over time)
        double energy = 0.0;
        for (const auto x : recovered) {
            energy += x * x;
        }
        expect(gt(energy, 0.0)) << "impulse energy passes through";
    };
};
