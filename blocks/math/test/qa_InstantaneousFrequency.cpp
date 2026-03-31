#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/math/InstantaneousFrequency.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"InstantaneousFrequency"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    "first output is zero (no prior sample)"_test = [] {
        InstantaneousFrequency<std::complex<float>> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        expect(eq(block.processOne(std::complex<float>{1.f, 0.f}), 0.f));
    };

    "constant complex sinusoid at known frequency"_test = []<typename T> {
        using CT  = std::complex<T>;
        using VT  = T;
        constexpr VT kFs   = VT{1000};
        constexpr VT kFreq = VT{100};

        InstantaneousFrequency<CT> block({});
        block.settings().init();
        std::ignore      = block.settings().applyStagedParameters();
        block.sample_rate = kFs;

        // skip first output (prev = 0)
        std::ignore = block.processOne(CT{VT{1}, VT{0}});

        const std::size_t N = 50;
        for (std::size_t n = 1; n <= N; ++n) {
            const VT phase = VT{2} * std::numbers::pi_v<VT> * kFreq / kFs * static_cast<VT>(n);
            const CT x{std::cos(phase), std::sin(phase)};
            const VT f = block.processOne(x);
            expect(abs(f - kFreq) < static_cast<VT>(1e-2)) << "frequency estimate close to true frequency";
        }
    } | std::tuple<float, double>{};

    "negative frequency (clockwise rotation)"_test = [] {
        using CT  = std::complex<double>;
        using VT  = double;
        constexpr VT kFs   = 1000.0;
        constexpr VT kFreq = -200.0;

        InstantaneousFrequency<CT> block({});
        block.settings().init();
        std::ignore       = block.settings().applyStagedParameters();
        block.sample_rate = kFs;

        std::ignore = block.processOne(CT{1.0, 0.0}); // prime

        for (std::size_t n = 1; n <= 30; ++n) {
            const VT phase = 2.0 * std::numbers::pi * kFreq / kFs * static_cast<VT>(n);
            const CT x{std::cos(phase), std::sin(phase)};
            const VT f = block.processOne(x);
            expect(abs(f - kFreq) < 1e-6) << "negative frequency estimate";
        }
    };
};
