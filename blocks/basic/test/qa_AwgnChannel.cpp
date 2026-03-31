#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <vector>

#include <gnuradio-4.0/basic/AwgnChannel.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"AwgnChannel"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "zero noise passes signal unchanged"_test = []<typename T> {
        AwgnChannel<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.noise_stddev = typename AwgnChannel<T>::value_type{0};
        block.seed         = 42ULL;
        block.settingsChanged({}, {});

        for (int i = 0; i < 8; ++i) {
            const T x = static_cast<T>(i + 1);
            expect(eq(block.processOne(x), x)) << "zero noise: output = input";
        }
    } | std::tuple<float, double>{};

    "noise standard deviation matches expected empirically"_test = [] {
        AwgnChannel<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.noise_stddev = 1.0;
        block.seed         = 12345ULL;
        block.settingsChanged({}, {});

        // Estimate noise variance from N samples
        const int N = 10000;
        double    sumSq = 0.0;
        for (int i = 0; i < N; ++i) {
            const double noise = block.processOne(0.0); // input=0 → output = noise only
            sumSq += noise * noise;
        }
        const double variance = sumSq / static_cast<double>(N);
        expect(approx(variance, 1.0, 0.1)) << "variance ≈ 1.0";
    };

    "complex noise has correct total power"_test = [] {
        AwgnChannel<std::complex<float>> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.noise_stddev = 1.f;
        block.seed         = 99ULL;
        block.settingsChanged({}, {});

        const int N = 10000;
        float     totalPower = 0.f;
        for (int i = 0; i < N; ++i) {
            const std::complex<float> n = block.processOne(std::complex<float>{});
            totalPower += n.real() * n.real() + n.imag() * n.imag();
        }
        const float variance = totalPower / static_cast<float>(N);
        expect(approx(static_cast<double>(variance), 1.0, 0.15)) << "complex noise total power ≈ σ²";
    };

    "same seed produces same sequence"_test = [] {
        AwgnChannel<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.noise_stddev = 0.5f;
        block.seed         = 7ULL;
        block.settingsChanged({}, {});

        std::vector<float> run1(8), run2(8);
        for (auto& v : run1) v = block.processOne(0.f);
        block.settingsChanged({}, {}); // re-seed
        for (auto& v : run2) v = block.processOne(0.f);

        expect(run1 == run2) << "same seed → same sequence";
    };
};
