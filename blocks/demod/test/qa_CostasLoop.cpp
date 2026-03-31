#include <boost/ut.hpp>
#include <complex>
#include <cmath>
#include <numbers>

#include <gnuradio-4.0/demod/CostasLoop.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"CostasLoop"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::demod;

    "BPSK order=2 settingsChanged"_test = []<typename T> {
        CostasLoop<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.loop_bandwidth = static_cast<T>(0.01);
        block.order          = 2U;
        block.settingsChanged({}, {});

        expect(block._alpha > T{}) << "alpha > 0";
        expect(block._beta  > T{}) << "beta > 0";
    } | std::tuple<float, double>{};

    "QPSK order=4 processes without crash"_test = [] {
        CostasLoop<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.order = 4U;
        block.settingsChanged({}, {});

        for (std::size_t n = 0; n < 50; ++n) {
            const float theta = 2.f * std::numbers::pi_v<float> * 0.05f * static_cast<float>(n);
            const auto  x     = std::complex<float>{std::cos(theta), std::sin(theta)};
            const auto  y     = block.processOne(x);
            expect(std::isfinite(y.real()) && std::isfinite(y.imag()));
        }
    };
};
