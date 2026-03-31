#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/filter/Squelch.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Squelch"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    auto makeBlock = []<typename T>(typename Squelch<T>::value_type threshold, gr::Size_t windowSize,
                                    gr::Size_t attack = 1U, gr::Size_t decay = 1U) {
        Squelch<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.threshold    = threshold;
        block.window_size  = windowSize;
        block.attack_length = attack;
        block.decay_length  = decay;
        block.settingsChanged({}, {});
        return block;
    };

    "strong signal passes through unchanged"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(static_cast<typename Squelch<T>::value_type>(0.01), 4U);

        // signal power = 1 >> threshold 0.01 → gate should open quickly
        const std::size_t N = 16;
        std::vector<T>    in(N, T{1});
        std::vector<T>    out(N);
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        // After a few warm-up samples the gate should be open
        expect(out[N - 1] != T{}) << "signal passes when power > threshold";
    } | std::tuple<float, double>{};

    "noise below threshold is gated to zero"_test = [&makeBlock]<typename T> {
        // very high threshold → gate stays closed
        auto block = makeBlock.template operator()<T>(static_cast<typename Squelch<T>::value_type>(1e6), 4U);

        const std::size_t N = 8;
        std::vector<T>    in(N, T{1});
        std::vector<T>    out(N);
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        for (std::size_t i = 0; i < N; ++i) {
            expect(eq(out[i], T{})) << "output zero when below threshold";
        }
    } | std::tuple<float, double>{};

    "attack_length delays gate opening"_test = [&makeBlock] {
        // attack=4: gate opens only after 4 consecutive above-threshold samples
        auto block = makeBlock.template operator()<float>(0.1f, 2U, 4U, 1U);

        std::vector<float> in(8, 1.f); // all above threshold
        std::vector<float> out(8);
        std::ignore = block.processBulk(std::span<const float>{in}, std::span<float>{out});

        // first 3 outputs should be zero (gate not yet open)
        for (std::size_t i = 0; i < 3U; ++i) {
            expect(eq(out[i], 0.f)) << "gate not open before attack at i=" << i;
        }
        // after attack_length=4, gate opens
        expect(out[3] != 0.f) << "gate opens at sample 3 (0-indexed)";
    };

    "decay_length delays gate closing"_test = [&makeBlock] {
        // decay=4: gate closes only after 4 consecutive below-threshold samples
        auto block = makeBlock.template operator()<float>(0.1f, 2U, 1U, 4U);

        // Open the gate first
        std::vector<float> strongIn(8, 1.f);
        std::vector<float> strongOut(8);
        std::ignore = block.processBulk(std::span<const float>{strongIn}, std::span<float>{strongOut});
        expect(strongOut[7] != 0.f) << "gate open after strong signal";

        // Then send silence (zero power)
        std::vector<float> silIn(8, 0.f);
        std::vector<float> silOut(8);
        std::ignore = block.processBulk(std::span<const float>{silIn}, std::span<float>{silOut});

        // gate should stay open for first 3 silence samples (decay not elapsed)
        for (std::size_t i = 0; i < 3U; ++i) {
            expect(eq(silOut[i], 0.f)) << "output is zero (gate open but input=0) at i=" << i;
        }
    };

    "settingsChanged resets all state"_test = [&makeBlock] {
        auto block = makeBlock.template operator()<double>(0.01, 4U, 1U, 1U);

        // drive to open state
        std::vector<double> in(8, 1.0);
        std::vector<double> out(8);
        std::ignore = block.processBulk(std::span<const double>{in}, std::span<double>{out});
        expect(block._open) << "gate open before reset";

        block.settingsChanged({}, {});
        expect(!block._open) << "gate closed after settingsChanged";
    };
};
