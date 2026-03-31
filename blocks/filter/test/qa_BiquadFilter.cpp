#include <boost/ut.hpp>
#include <cmath>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/filter/BiquadFilter.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"BiquadFilter"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    "identity filter (b0=1, all others 0) passes through unchanged"_test = []<typename T> {
        BiquadFilter<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        // defaults: b0=1, rest=0 → identity

        for (int i = 0; i < 10; ++i) {
            const T x = static_cast<T>(i);
            expect(approx(static_cast<double>(block.processOne(x)), static_cast<double>(x), 1e-9));
        }
    } | std::tuple<float, double>{};

    "pure gain (b0=2) doubles all samples"_test = [] {
        BiquadFilter<float> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.b0 = 2.f;

        expect(approx(static_cast<double>(block.processOne(3.f)), 6.0, 1e-6));
        expect(approx(static_cast<double>(block.processOne(5.f)), 10.0, 1e-6));
    };

    "first-order low-pass: DC gain equals 1"_test = [] {
        // y[n] = b0*x[n] + b1*x[n-1] — a1*y[n-1]
        // First-order Butterworth LP at Fs/4: b0=b1=0.5, a1=-0.0 (matched filter approach)
        // Use integrating section instead: b0=0.5, b1=0.5, a1=0, a2=0 → DC gain=1
        BiquadFilter<double> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.b0 = 0.5;
        block.b1 = 0.5;
        block.a1 = 0.0;
        block.a2 = 0.0;

        // drive to steady state with DC=1
        double out = 0.0;
        for (int i = 0; i < 50; ++i) {
            out = block.processOne(1.0);
        }
        expect(approx(out, 1.0, 1e-9)) << "DC gain = 1";
    };

    "state resets between independent block instances"_test = [] {
        BiquadFilter<float> a({});
        a.settings().init();
        std::ignore = a.settings().applyStagedParameters();
        a.b0 = 1.f; a.b1 = 0.5f;

        BiquadFilter<float> b({});
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.b0 = 1.f; b.b1 = 0.5f;

        std::ignore = a.processOne(1.f); // drive state in a
        const float ya2 = a.processOne(0.f);
        const float yb1 = b.processOne(0.f); // b starts fresh
        expect(ya2 != yb1) << "independent state";
    };
};
