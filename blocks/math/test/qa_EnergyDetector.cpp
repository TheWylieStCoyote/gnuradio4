#include <boost/ut.hpp>
#include <complex>

#include <gnuradio-4.0/math/EnergyDetector.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"EnergyDetector"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    "samples pass through unchanged"_test = []<typename T> {
        EnergyDetector<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        expect(approx(static_cast<double>(block.processOne(T{3})), 3.0, 1e-9)) << "pass-through";
        expect(approx(static_cast<double>(block.processOne(T{-5})), -5.0, 1e-9)) << "negative pass-through";
    } | kTypes;

    "rising edge detection: tag published when energy crosses threshold"_test = []<typename T> {
        EnergyDetector<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.window_size  = gr::Size_t{4U};
        block.threshold    = static_cast<T>(4);  // threshold = 4 (energy of 4 unit samples)
        block.hysteresis   = static_cast<T>(0);
        block.tag_key      = "det";
        block.settingsChanged({}, {{"window_size", gr::Size_t{4U}}});

        // Feed zeros — no detection
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(T{0});
        }
        expect(!block._detected) << "no detection in silence";

        // Feed enough signal to cross threshold: 4 samples of amplitude 2 → energy = 4*4 = 16 > 4
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(T{2});
        }
        expect(block._detected) << "detected after energy exceeds threshold";
    } | kTypes;

    "falling edge: detection clears when energy drops below threshold"_test = []<typename T> {
        EnergyDetector<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.window_size  = gr::Size_t{4U};
        block.threshold    = static_cast<T>(4);
        block.hysteresis   = static_cast<T>(0);
        block.tag_key      = "det";
        block.settingsChanged({}, {{"window_size", gr::Size_t{4U}}});

        // Trigger detection
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(T{2});
        }
        expect(block._detected) << "pre-condition: detected";

        // Feed zeros until window empties — energy falls to 0 < threshold
        for (int i = 0; i < 4; ++i) {
            std::ignore = block.processOne(T{0});
        }
        expect(!block._detected) << "detection cleared after energy drops";
    } | kTypes;

    "hysteresis prevents re-trigger on marginal signal"_test = [] {
        using T = double;
        EnergyDetector<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.window_size  = gr::Size_t{1U};
        block.threshold    = T{1};
        block.hysteresis   = T{0.5};  // upper=1.5, lower=0.5
        block.tag_key      = "det";
        block.settingsChanged({}, {{"window_size", gr::Size_t{1U}}});

        // energy = 1.2 — above threshold (1) but below upper (1.5) — no trigger
        std::ignore = block.processOne(T{std::sqrt(1.2)});
        expect(!block._detected) << "below upper hysteresis band";

        // energy = 2 — above upper (1.5) — triggers
        std::ignore = block.processOne(T{std::sqrt(2.0)});
        expect(block._detected) << "above upper hysteresis, detected";

        // energy = 0.8 — below threshold (1) but above lower (0.5) — stays detected
        std::ignore = block.processOne(T{std::sqrt(0.8)});
        expect(block._detected) << "above lower hysteresis, still detected";

        // energy = 0.2 — below lower (0.5) — clears
        std::ignore = block.processOne(T{std::sqrt(0.2)});
        expect(!block._detected) << "below lower hysteresis, detection cleared";
    };

    "complex input: energy is |x|^2"_test = [] {
        using T  = std::complex<double>;
        EnergyDetector<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.window_size  = gr::Size_t{1U};
        block.threshold    = 1.0;
        block.hysteresis   = 0.0;
        block.tag_key      = "det";
        block.settingsChanged({}, {{"window_size", gr::Size_t{1U}}});

        // |3+4j|^2 = 25 > 1 — should trigger
        std::ignore = block.processOne(T{3.0, 4.0});
        expect(block._detected) << "complex energy > threshold";
    };

    "settings reset clears detection state"_test = []<typename T> {
        EnergyDetector<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.window_size  = gr::Size_t{1U};
        block.threshold    = static_cast<T>(0.5);
        block.hysteresis   = static_cast<T>(0);
        block.tag_key      = "det";
        block.settingsChanged({}, {{"window_size", gr::Size_t{1U}}});

        std::ignore = block.processOne(T{2});  // trigger
        expect(block._detected) << "pre-condition";

        block.settingsChanged({}, {});  // reset
        expect(!block._detected) << "detection cleared on settings reset";
    } | kTypes;
};
