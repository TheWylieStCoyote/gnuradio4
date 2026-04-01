#include <boost/ut.hpp>
#include <complex>
#include <vector>

#include <gnuradio-4.0/fourier/PolyphaseChannelizer.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"PolyphaseChannelizer"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fourier;

    auto makeBlock = [](gr::Size_t nCh, std::vector<float> userTaps = {}) {
        PolyphaseChannelizer<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_channels = nCh;
        if (!userTaps.empty()) b.taps = userTaps;
        b.settingsChanged({}, {});
        return b;
    };

    "settingsChanged resizes output ports to n_channels"_test = [&makeBlock] {
        auto b = makeBlock(4U);
        expect(eq(b.out.size(), std::size_t{4}));
    };

    "input_chunk_size equals n_channels"_test = [&makeBlock] {
        auto b = makeBlock(8U);
        expect(eq(static_cast<std::size_t>(b.input_chunk_size), std::size_t{8}));
    };

    "output_chunk_size is 1"_test = [&makeBlock] {
        auto b = makeBlock(4U);
        expect(eq(static_cast<std::size_t>(b.output_chunk_size), std::size_t{1}));
    };

    "user-supplied taps are decomposed into n_channels phases"_test = [&makeBlock] {
        // 8 taps, 4 channels → 2 taps per phase
        const std::vector<float> h = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
        auto b = makeBlock(4U, h);
        expect(eq(b._nPerPhase, std::size_t{2}));
        // phase[0] = h[0], h[4] = 1, 5
        expect(eq(b._phases[0][0], 1.0f));
        expect(eq(b._phases[0][1], 5.0f));
        // phase[1] = h[1], h[5] = 2, 6
        expect(eq(b._phases[1][0], 2.0f));
        expect(eq(b._phases[1][1], 6.0f));
        // phase[3] = h[3], h[7] = 4, 8
        expect(eq(b._phases[3][0], 4.0f));
        expect(eq(b._phases[3][1], 8.0f));
    };

    "history buffer size equals n_channels * nPerPhase"_test = [&makeBlock] {
        const std::vector<float> h = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
        auto b = makeBlock(4U, h);
        expect(eq(b._history.size(), std::size_t{8})); // 4 * 2
    };

    "history initialised to zero after settingsChanged"_test = [&makeBlock] {
        auto b = makeBlock(4U);
        for (const float v : b._history) expect(eq(v, 0.0f));
    };

    "settingsChanged resets history to zero"_test = [&makeBlock] {
        auto b = makeBlock(4U);
        // corrupt history manually
        for (auto& v : b._history) v = 99.0f;
        b.settingsChanged({}, {});
        for (const float v : b._history) expect(eq(v, 0.0f));
    };

    "auto-designed prototype produces nPerPhase > 0"_test = [&makeBlock] {
        auto b = makeBlock(8U); // no user taps → auto-design
        expect(b._nPerPhase > 0UZ) << "auto-designed prototype should have at least one tap per phase";
    };

    "complex<float> compiles and initialises"_test = [] {
        PolyphaseChannelizer<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_channels = 2U;
        b.taps = std::vector<float>{0.5f, 0.5f};
        b.settingsChanged({}, {});
        expect(eq(b.out.size(), std::size_t{2}));
        expect(eq(b._nPerPhase, std::size_t{1}));
    };

    "double type compiles and initialises"_test = [] {
        PolyphaseChannelizer<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.n_channels = 4U;
        b.settingsChanged({}, {});
        expect(eq(b.out.size(), std::size_t{4}));
    };

    "n_channels=1 is a trivial pass-through config"_test = [&makeBlock] {
        auto b = makeBlock(1U);
        expect(eq(b.out.size(), std::size_t{1}));
        expect(eq(static_cast<std::size_t>(b.input_chunk_size), std::size_t{1}));
    };
};
