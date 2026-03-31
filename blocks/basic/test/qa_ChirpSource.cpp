#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/basic/ChirpSource.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"ChirpSource"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "output is bounded by amplitude"_test = []<typename T> {
        ChirpSource<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.f_start      = T{100};
        block.f_stop       = T{1000};
        block.sample_rate  = T{8000};
        block.sweep_length = 256U;
        block.amplitude    = T{2};
        block.settingsChanged({}, {});

        for (int i = 0; i < 512; ++i) {
            const T y = block.processOne();
            expect(y >= static_cast<T>(-2.01) && y <= static_cast<T>(2.01)) << "amplitude bounded at i=" << i;
        }
    } | std::tuple<float, double>{};

    "settingsChanged resets phase and sample counter"_test = [] {
        ChirpSource<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.f_start      = 0.f;
        block.f_stop       = 0.f; // constant 0 Hz → constant output sin(0)=0
        block.sample_rate  = 1.f;
        block.sweep_length = 64U;
        block.settingsChanged({}, {});

        const float y0 = block.processOne(); // sin(0) = 0
        expect(approx(static_cast<double>(y0), 0.0, 1e-5)) << "first sample at phase=0";

        block.settingsChanged({}, {}); // reset
        const float y1 = block.processOne();
        expect(approx(static_cast<double>(y1), 0.0, 1e-5)) << "resets to phase=0";
    };

    "n counter wraps after sweep_length samples"_test = [] {
        ChirpSource<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.f_start      = 100.0;
        block.f_stop       = 200.0;
        block.sample_rate  = 8000.0;
        block.sweep_length = 16U;
        block.amplitude    = 1.0;
        block.settingsChanged({}, {});

        // After sweep_length samples, _n should reset to 0
        for (int i = 0; i < 16; ++i) std::ignore = block.processOne();
        expect(eq(block._n, std::size_t{0})) << "_n wraps back to 0";

        // After 32 samples, _n should be 0 again
        for (int i = 0; i < 16; ++i) std::ignore = block.processOne();
        expect(eq(block._n, std::size_t{0})) << "_n wraps again";
    };
};
