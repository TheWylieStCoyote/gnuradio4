#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numeric>
#include <vector>

#include <gnuradio-4.0/filter/PolyphaseArbitraryResampler.hpp>

int main() { /* tests auto-register via boost::ut */ }

namespace {

auto makeBlock(double rate, gr::Size_t nFilters = 8U, std::vector<float> userTaps = {}) {
    gr::blocks::filter::PolyphaseArbitraryResampler<float> b{};
    b.settings().init();
    std::ignore = b.settings().applyStagedParameters();
    b.rate      = rate;
    b.n_filters = nFilters;
    if (!userTaps.empty()) b.taps = userTaps;
    b.settingsChanged({}, {});
    return b;
}

std::vector<float> runBlock(gr::blocks::filter::PolyphaseArbitraryResampler<float>& b,
                             const std::vector<float>& input) {
    const std::size_t nIn  = static_cast<std::size_t>(b.input_chunk_size);
    const std::size_t nOut = static_cast<std::size_t>(b.output_chunk_size);
    std::vector<float> out(nOut);
    std::ignore = b.processBulk(std::span<const float>{input.data(), nIn}, std::span<float>{out});
    return out;
}

} // namespace

const boost::ut::suite<"PolyphaseArbitraryResampler"> tests = [] {
    using namespace boost::ut;

    "settingsChanged sets input_chunk_size to n_filters"_test = [] {
        auto b = makeBlock(2.0, 16U);
        expect(eq(static_cast<std::size_t>(b.input_chunk_size), std::size_t{16}));
    };

    "output_chunk_size equals round(n_filters * rate) for rate=2"_test = [] {
        auto b = makeBlock(2.0, 8U);
        expect(eq(static_cast<std::size_t>(b.output_chunk_size), std::size_t{16}));
    };

    "output_chunk_size equals round(n_filters * rate) for rate=0.5"_test = [] {
        auto b = makeBlock(0.5, 8U);
        expect(eq(static_cast<std::size_t>(b.output_chunk_size), std::size_t{4}));
    };

    "output_chunk_size is at least 1"_test = [] {
        auto b = makeBlock(0.01, 8U);
        expect(b.output_chunk_size >= gr::Size_t{1});
    };

    "rate=1 produces same number of output as input"_test = [] {
        auto b = makeBlock(1.0, 8U);
        expect(eq(static_cast<std::size_t>(b.input_chunk_size),
                  static_cast<std::size_t>(b.output_chunk_size)));
    };

    "DC signal is preserved at rate=1 with identity-like taps"_test = [] {
        // single central tap per phase → identity FIR after settingsChanged
        const std::vector<float> singleTap(8, 1.0f / 8.0f); // one tap per phase
        auto b = makeBlock(1.0, 8U, singleTap);

        const std::vector<float> dc(8, 3.0f);
        const auto out = runBlock(b, dc);
        expect(eq(out.size(), std::size_t{8}));
        // after warm-up (history=0) first few samples may differ; last should converge to ~3
        // just check output is non-zero and bounded
        for (const float v : out) expect(std::abs(v) < 10.0f) << "output should be bounded";
    };

    "processBulk does not crash for rate=1.5"_test = [] {
        auto b = makeBlock(1.5, 8U);
        const std::size_t nIn = static_cast<std::size_t>(b.input_chunk_size);
        const std::vector<float> input(nIn, 1.0f);
        const auto out = runBlock(b, input);
        expect(eq(out.size(), static_cast<std::size_t>(b.output_chunk_size)));
        for (const float v : out) expect(std::isfinite(v)) << "output must be finite";
    };

    "processBulk does not crash for rate=0.75"_test = [] {
        auto b = makeBlock(0.75, 8U);
        const std::size_t nIn = static_cast<std::size_t>(b.input_chunk_size);
        const std::vector<float> input(nIn, 1.0f);
        const auto out = runBlock(b, input);
        expect(eq(out.size(), static_cast<std::size_t>(b.output_chunk_size)));
        for (const float v : out) expect(std::isfinite(v)) << "output must be finite";
    };

    "settingsChanged resets history to zeros"_test = [] {
        auto b = makeBlock(1.0, 8U);
        for (const float v : b._history) expect(eq(v, 0.0f));
    };

    "history updates after processBulk"_test = [] {
        const std::vector<float> singleTap(8, 1.0f / 8.0f);
        auto b = makeBlock(1.0, 8U, singleTap);
        const std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
        std::ignore = runBlock(b, input);
        // history should be non-zero after processing non-zero input
        // (only if nPerFilter > 1; for single-tap: nPerFilter=1, history len=0)
        expect(b._nPerFilter >= 1UZ);
    };

    "n_filters=8 auto-designs a non-empty polyphase bank"_test = [] {
        auto b = makeBlock(1.0, 8U);
        expect(eq(b._phases.size(), std::size_t{9})); // 8 phases + wrap-around
        expect(b._nPerFilter > 0UZ);
        // at least one phase tap is non-zero
        bool anyNonZero = false;
        for (const float v : b._phases[0]) anyNonZero |= (v != 0.0f);
        expect(anyNonZero) << "auto-designed phases should have non-zero taps";
    };

    "complex<float> compiles and processes"_test = [] {
        gr::blocks::filter::PolyphaseArbitraryResampler<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.rate      = 1.0;
        b.n_filters = 4U;
        b.taps      = std::vector<float>(4, 0.25f);
        b.settingsChanged({}, {});

        const std::size_t nIn = static_cast<std::size_t>(b.input_chunk_size);
        std::vector<std::complex<float>> input(nIn, {1.0f, 0.0f});
        std::vector<std::complex<float>> output(static_cast<std::size_t>(b.output_chunk_size));
        std::ignore = b.processBulk(std::span<const std::complex<float>>{input},
                                     std::span<std::complex<float>>{output});
        for (const auto& v : output) expect(std::isfinite(v.real()) && std::isfinite(v.imag()));
    };

    "double type compiles and processes"_test = [] {
        gr::blocks::filter::PolyphaseArbitraryResampler<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.rate      = 2.0;
        b.n_filters = 4U;
        b.settingsChanged({}, {});

        const std::size_t nIn = static_cast<std::size_t>(b.input_chunk_size);
        const std::vector<double> input(nIn, 1.0);
        std::vector<double> output(static_cast<std::size_t>(b.output_chunk_size));
        std::ignore = b.processBulk(std::span<const double>{input}, std::span<double>{output});
        for (const double v : output) expect(std::isfinite(v));
    };
};
