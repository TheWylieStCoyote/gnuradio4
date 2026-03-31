#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>
#include <numeric>
#include <vector>

#include <gnuradio-4.0/filter/RationalResampler.hpp>

namespace {

gr::blocks::filter::RationalResampler<float> makeResampler(gr::Size_t L, gr::Size_t M) {
    gr::blocks::filter::RationalResampler<float> r{};
    r.settings().init();
    std::ignore = r.settings().applyStagedParameters();
    r.interpolation = L;
    r.decimation    = M;
    r.settingsChanged({}, {});
    return r;
}

// Run nBlocks chunks of M→L samples through the resampler and collect outputs.
std::vector<float> runBlocks(gr::blocks::filter::RationalResampler<float>& r,
                              const std::vector<float>& signal, std::size_t nBlocks) {
    const std::size_t M = static_cast<std::size_t>(r.input_chunk_size);
    const std::size_t L = static_cast<std::size_t>(r.output_chunk_size);
    std::vector<float> out;
    out.reserve(nBlocks * L);
    std::vector<float> outBuf(L);
    for (std::size_t b = 0; b < nBlocks; ++b) {
        const std::size_t inOff = b * M;
        std::ignore = r.processBulk(
            std::span<const float>{signal.data() + inOff, M},
            std::span<float>{outBuf});
        out.insert(out.end(), outBuf.begin(), outBuf.end());
    }
    return out;
}

} // namespace

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"RationalResampler"> tests = [] {
    using namespace boost::ut;

    "chunk sizes reflect reduced L and M"_test = [] {
        auto r = makeResampler(4U, 6U); // reduces to 2/3
        expect(eq(static_cast<std::size_t>(r.input_chunk_size),  std::size_t{3}));
        expect(eq(static_cast<std::size_t>(r.output_chunk_size), std::size_t{2}));
    };

    "upsample 2:1 chunk sizes"_test = [] {
        auto r = makeResampler(2U, 1U);
        expect(eq(static_cast<std::size_t>(r.input_chunk_size),  std::size_t{1}));
        expect(eq(static_cast<std::size_t>(r.output_chunk_size), std::size_t{2}));
    };

    "decimate 1:2 chunk sizes"_test = [] {
        auto r = makeResampler(1U, 2U);
        expect(eq(static_cast<std::size_t>(r.input_chunk_size),  std::size_t{2}));
        expect(eq(static_cast<std::size_t>(r.output_chunk_size), std::size_t{1}));
    };

    "identity 1:1 DC passes through at steady state"_test = [] {
        auto r = makeResampler(1U, 1U);
        constexpr std::size_t nBlocks = 300UZ;
        std::vector<float> signal(nBlocks, 1.0f);
        auto out = runBlocks(r, signal, nBlocks);

        // skip warm-up; mean of the steady-state region should be ≈ 1
        const std::size_t warmup = 100UZ;
        const float mean = std::accumulate(out.begin() + static_cast<std::ptrdiff_t>(warmup),
                                            out.end(), 0.0f) / static_cast<float>(nBlocks - warmup);
        expect(std::abs(mean - 1.0f) < 0.02f) << "DC gain ≈ 1 for identity";
    };

    "upsample 2:1 DC is preserved at steady state"_test = [] {
        auto r = makeResampler(2U, 1U);
        constexpr std::size_t nBlocks = 300UZ;
        std::vector<float> signal(nBlocks, 1.0f); // 1 sample per block consumed
        auto out = runBlocks(r, signal, nBlocks);  // 2 samples per block produced

        const std::size_t warmup = 100UZ * 2UZ; // warmup in output samples
        const float mean = std::accumulate(out.begin() + static_cast<std::ptrdiff_t>(warmup),
                                            out.end(), 0.0f) / static_cast<float>(out.size() - warmup);
        expect(std::abs(mean - 1.0f) < 0.05f) << "DC preserved after 2:1 upsample";
    };

    "rational 3:2 chunk sizes are correct"_test = [] {
        auto r = makeResampler(3U, 2U);
        expect(eq(static_cast<std::size_t>(r.input_chunk_size),  std::size_t{2}));
        expect(eq(static_cast<std::size_t>(r.output_chunk_size), std::size_t{3}));
    };

    "settingsChanged resets history and updates chunk sizes"_test = [] {
        auto r = makeResampler(2U, 1U);

        std::vector<float> in1(1, 1.0f);
        std::vector<float> out1(2);
        std::ignore = r.processBulk(std::span<const float>{in1}, std::span<float>{out1});

        r.interpolation = 3U;
        r.decimation    = 1U;
        r.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(r.input_chunk_size),  std::size_t{1}));
        expect(eq(static_cast<std::size_t>(r.output_chunk_size), std::size_t{3}));
    };

    "complex<float> 2:1 upsample runs without error"_test = [] {
        gr::blocks::filter::RationalResampler<std::complex<float>> r{};
        r.settings().init();
        std::ignore = r.settings().applyStagedParameters();
        r.interpolation = 2U;
        r.decimation    = 1U;
        r.settingsChanged({}, {});

        std::vector<std::complex<float>> in(1, {1.0f, 0.0f});
        std::vector<std::complex<float>> out(2);
        std::ignore = r.processBulk(std::span<const std::complex<float>>{in},
                                     std::span<std::complex<float>>{out});
        expect(true);
    };

    "user-supplied taps are used when provided"_test = [] {
        gr::blocks::filter::RationalResampler<float> r{};
        r.settings().init();
        std::ignore = r.settings().applyStagedParameters();
        r.interpolation = 1U;
        r.decimation    = 1U;
        r.taps          = std::vector<float>{1.0f}; // single tap = identity
        r.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(r.input_chunk_size),  std::size_t{1}));
        expect(eq(static_cast<std::size_t>(r.output_chunk_size), std::size_t{1}));

        std::vector<float> in{0.5f};
        std::vector<float> out(1);
        std::ignore = r.processBulk(std::span<const float>{in}, std::span<float>{out});
        expect(std::abs(out[0] - 0.5f) < 1e-5f) << "single-tap identity preserves value";
    };
};
