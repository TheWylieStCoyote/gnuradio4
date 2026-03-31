#include <boost/ut.hpp>
#include <complex>
#include <cmath>

#include <gnuradio-4.0/math/AgcBlock.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"AgcBlock"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    "gain rises when output is below target"_test = []<typename T> {
        AgcBlock<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.target_power = static_cast<T>(1);
        block.attack_rate  = static_cast<T>(0.1);
        block.decay_rate   = static_cast<T>(0.1);
        block.max_gain     = static_cast<T>(1000);
        block.min_gain     = static_cast<T>(1e-6);
        block._gain        = static_cast<T>(0.01);

        const T input{2};
        T       prev_out{};
        for (int i = 0; i < 60; ++i) {
            prev_out = block.processOne(input);
        }
        const double final_power = static_cast<double>(prev_out) * static_cast<double>(prev_out);
        expect(gt(final_power, 0.5)) << "gain converged upward";
    } | kTypes;

    "gain decreases when output exceeds target"_test = []<typename T> {
        AgcBlock<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.target_power = static_cast<T>(1);
        block.attack_rate  = static_cast<T>(0.1);
        block.decay_rate   = static_cast<T>(0.1);
        block.max_gain     = static_cast<T>(1000);
        block.min_gain     = static_cast<T>(1e-6);
        block._gain        = static_cast<T>(100);

        const T initial_gain = block._gain;
        std::ignore           = block.processOne(T{1});
        expect(lt(static_cast<double>(block._gain), static_cast<double>(initial_gain))) << "gain decreased";
    } | kTypes;

    "gain is clamped to [min_gain, max_gain]"_test = []<typename T> {
        AgcBlock<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.target_power = static_cast<T>(1);
        block.attack_rate  = static_cast<T>(0.5);
        block.decay_rate   = static_cast<T>(0.5);
        block.max_gain     = static_cast<T>(10);
        block.min_gain     = static_cast<T>(0.1);

        for (int i = 0; i < 100; ++i) {
            std::ignore = block.processOne(T{0});
        }
        expect(le(static_cast<double>(block._gain), static_cast<double>(block.max_gain))) << "max_gain enforced";

        block._gain = block.min_gain;
        for (int i = 0; i < 100; ++i) {
            std::ignore = block.processOne(T{1000});
        }
        expect(ge(static_cast<double>(block._gain), static_cast<double>(block.min_gain))) << "min_gain enforced";
    } | kTypes;

    "complex: unity-magnitude phasor converges to target RMS"_test = [] {
        using T  = std::complex<double>;
        using VT = double;
        AgcBlock<T> block({});
        block.settings().init();
        std::ignore        = block.settings().applyStagedParameters();
        block.target_power = VT{4};   // target RMS² = 4 → target amplitude = 2
        block.attack_rate  = VT{0.05};
        block.decay_rate   = VT{0.05};
        block.max_gain     = VT{100};
        block.min_gain     = VT{1e-6};
        block._gain        = VT{1};

        T last{};
        for (int i = 0; i < 200; ++i) {
            last = block.processOne(T{1.0, 0.0});
        }
        expect(approx(std::abs(last), 2.0, 0.3)) << "complex AGC converges";
    };
};
