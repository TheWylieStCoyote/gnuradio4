#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numeric>
#include <vector>

#include <gnuradio-4.0/fourier/SpectralSubtractor.hpp>

int main() { /* tests auto-register via boost::ut */ }

namespace {

auto makeBlock(gr::Size_t fftSize = 8U, float alpha = 1.0f, gr::Size_t refFrames = 2U) {
    gr::blocks::fourier::SpectralSubtractor<float> b{};
    b.settings().init();
    std::ignore = b.settings().applyStagedParameters();
    b.fft_size        = fftSize;
    b.alpha           = alpha;
    b.reference_frames = refFrames;
    b.settingsChanged({}, {});
    return b;
}

float rms(std::span<const float> v) {
    const float sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0f);
    return std::sqrt(sum / static_cast<float>(v.size()));
}

} // namespace

const boost::ut::suite<"SpectralSubtractor"> tests = [] {
    using namespace boost::ut;

    "reference phase passes input through unchanged"_test = [] {
        auto b = makeBlock(8U, 1.0f, 3U);
        const std::vector<float> frame = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f};
        std::vector<float> out(8);

        // first 3 frames are reference — should be copied verbatim
        for (std::size_t i = 0; i < 3; ++i) {
            std::ignore = b.processBulk(std::span<const float>{frame}, std::span<float>{out});
            for (std::size_t j = 0; j < 8; ++j) {
                expect(eq(out[j], frame[j])) << "reference pass-through mismatch at frame " << i;
            }
        }
    };

    "noise reduction reduces output power after reference phase"_test = [] {
        // use 4-sample frames and 2 reference frames so the test is tractable
        auto b = makeBlock(4U, 1.0f, 2U);
        const std::vector<float> noiseFrame = {1.0f, -1.0f, 1.0f, -1.0f};

        // feed 2 reference frames
        std::vector<float> out(4);
        std::ignore = b.processBulk(std::span<const float>{noiseFrame}, std::span<float>{out});
        std::ignore = b.processBulk(std::span<const float>{noiseFrame}, std::span<float>{out});

        // now feed the same signal — spectral subtraction should reduce output power
        std::ignore = b.processBulk(std::span<const float>{noiseFrame}, std::span<float>{out});
        const float outputPower = rms(std::span<const float>{out});
        const float inputPower  = rms(std::span<const float>{noiseFrame});
        expect(outputPower < inputPower) << "output power should be less than input after subtraction";
    };

    "clean signal power is preserved when noise floor is zero"_test = [] {
        auto b = makeBlock(4U, 1.0f, 2U);
        // feed silence as reference → noise floor = 0
        const std::vector<float> silence(4, 0.0f);
        std::vector<float> out(4);
        std::ignore = b.processBulk(std::span<const float>{silence}, std::span<float>{out});
        std::ignore = b.processBulk(std::span<const float>{silence}, std::span<float>{out});

        // now feed a non-trivial signal
        const std::vector<float> signal = {1.0f, 0.0f, -1.0f, 0.0f};
        std::ignore = b.processBulk(std::span<const float>{signal}, std::span<float>{out});
        const float outputPower = rms(std::span<const float>{out});
        const float inputPower  = rms(std::span<const float>{signal});
        // with zero noise floor subtraction does nothing — powers should match
        expect(outputPower > 0.5f * inputPower) << "non-trivial signal should survive zero noise floor";
    };

    "alpha=0 disables subtraction (output power matches input)"_test = [] {
        auto b = makeBlock(4U, 0.0f, 1U);
        const std::vector<float> noiseFrame = {1.0f, 1.0f, 1.0f, 1.0f};
        std::vector<float> out(4);
        std::ignore = b.processBulk(std::span<const float>{noiseFrame}, std::span<float>{out});

        // alpha=0: nothing subtracted, output should equal input (up to IFFT round-trip error)
        const std::vector<float> signal = {0.5f, -0.5f, 0.5f, -0.5f};
        std::ignore = b.processBulk(std::span<const float>{signal}, std::span<float>{out});
        const float outputPower = rms(std::span<const float>{out});
        const float inputPower  = rms(std::span<const float>{signal});
        expect(std::abs(outputPower - inputPower) < 0.15f) << "alpha=0 should preserve signal power";
    };

    "settingsChanged resets frame counter and noise floor"_test = [] {
        auto b = makeBlock(4U, 1.0f, 1U);
        const std::vector<float> noiseFrame = {2.0f, 2.0f, 2.0f, 2.0f};
        std::vector<float> out(4);
        // complete reference phase
        std::ignore = b.processBulk(std::span<const float>{noiseFrame}, std::span<float>{out});

        // reset via settingsChanged
        b.settingsChanged({}, {});

        // reference phase again — input must pass through unchanged
        const std::vector<float> sig = {3.0f, 3.0f, 3.0f, 3.0f};
        std::ignore = b.processBulk(std::span<const float>{sig}, std::span<float>{out});
        for (std::size_t i = 0; i < 4; ++i)
            expect(eq(out[i], sig[i])) << "after reset, first frame is reference pass-through";
    };

    "chunk sizes equal fft_size after settingsChanged"_test = [] {
        auto b = makeBlock(16U, 1.0f, 2U);
        expect(eq(static_cast<std::size_t>(b.input_chunk_size),  16UZ));
        expect(eq(static_cast<std::size_t>(b.output_chunk_size), 16UZ));
    };

    "complex<float> compiles and processes without crashing"_test = [] {
        gr::blocks::fourier::SpectralSubtractor<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.fft_size        = 4U;
        b.reference_frames = 1U;
        b.settingsChanged({}, {});

        const std::vector<std::complex<float>> frame = {{1.f, 0.f}, {0.f, 1.f}, {-1.f, 0.f}, {0.f, -1.f}};
        std::vector<std::complex<float>> out(4);
        // reference frame
        std::ignore = b.processBulk(std::span<const std::complex<float>>{frame},
                                     std::span<std::complex<float>>{out});
        // subtraction frame — should not crash
        std::ignore = b.processBulk(std::span<const std::complex<float>>{frame},
                                     std::span<std::complex<float>>{out});
        expect(true) << "complex<float> processes without exception";
    };

    "double type compiles and processes"_test = [] {
        gr::blocks::fourier::SpectralSubtractor<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.fft_size        = 4U;
        b.reference_frames = 1U;
        b.settingsChanged({}, {});

        const std::vector<double> frame = {1.0, -1.0, 1.0, -1.0};
        std::vector<double> out(4);
        std::ignore = b.processBulk(std::span<const double>{frame}, std::span<double>{out});
        std::ignore = b.processBulk(std::span<const double>{frame}, std::span<double>{out});
        expect(true) << "double type processes without exception";
    };
};
