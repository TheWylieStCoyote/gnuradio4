#include <boost/ut.hpp>
#include <complex>
#include <cmath>
#include <numbers>

#include <gnuradio-4.0/demod/PLL.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"PLL"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::demod;

    "settingsChanged computes nonzero alpha and beta"_test = []<typename T> {
        PLL<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.loop_bandwidth = static_cast<T>(0.01);
        block.damping        = static_cast<T>(0.707);
        block.settingsChanged({}, {});

        expect(block._alpha > T{}) << "alpha > 0";
        expect(block._beta  > T{}) << "beta > 0";
    } | std::tuple<float, double>{};

    "passes complex samples without crash"_test = [] {
        PLL<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.settingsChanged({}, {});

        constexpr std::size_t N = 100;
        for (std::size_t n = 0; n < N; ++n) {
            const float theta = 2.f * std::numbers::pi_v<float> * 0.1f * static_cast<float>(n);
            const std::complex<float> x{std::cos(theta), std::sin(theta)};
            const std::complex<float> y = block.processOne(x);
            expect(std::isfinite(y.real()) && std::isfinite(y.imag())) << "output must be finite";
        }
    };
};
