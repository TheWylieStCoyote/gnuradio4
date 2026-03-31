#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/basic/WindowApply.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"WindowApply"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "rectangular window is identity"_test = []<typename T> {
        WindowApply<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.frame_size  = 8U;
        block.window_type = "Rectangular";
        block.settingsChanged({}, {});

        const std::vector<T> in(8, T{1});
        std::vector<T>       out(8);
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        for (std::size_t i = 0; i < 8; ++i) {
            expect(approx(static_cast<double>(out[i]), 1.0, 1e-6)) << "rectangular window at i=" << i;
        }
    } | std::tuple<float, double>{};

    "Hann window tapers ends to zero"_test = [] {
        WindowApply<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.frame_size  = 16U;
        block.window_type = "Hann";
        block.settingsChanged({}, {});

        const std::vector<float> in(16, 1.f);
        std::vector<float>       out(16);
        std::ignore = block.processBulk(std::span<const float>{in}, std::span<float>{out});

        // Hann window: first and last samples should be near zero
        expect(approx(static_cast<double>(out[0]),  0.0, 0.05)) << "Hann start ≈ 0";
        expect(approx(static_cast<double>(out[15]), 0.0, 0.05)) << "Hann end ≈ 0";
        // Middle sample should be near 1 (Hann peak)
        expect(out[7] > 0.5f) << "Hann peak > 0.5";
    };

    "window coefficients resize with frame_size"_test = [] {
        WindowApply<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.frame_size  = 32U;
        block.window_type = "Hamming";
        block.settingsChanged({}, {});

        expect(eq(block._window.size(), std::size_t{32})) << "window size matches frame_size";
    };
};
