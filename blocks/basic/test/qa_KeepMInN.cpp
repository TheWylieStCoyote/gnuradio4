#include <boost/ut.hpp>
#include <vector>

#include <gnuradio-4.0/basic/KeepMInN.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"KeepMInN"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    "keeps first M of N unchanged"_test = []<typename T> {
        KeepMInN<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.m_samples = 2U;
        block.n_samples = 4U;
        block.settingsChanged({}, {});

        // Input one group of N=4
        const std::vector<T> in{static_cast<T>(1), static_cast<T>(2), static_cast<T>(3), static_cast<T>(4)};
        std::vector<T>       out(2);
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        expect(eq(out[0], static_cast<T>(1))) << "keeps sample 0";
        expect(eq(out[1], static_cast<T>(2))) << "keeps sample 1";
    } | std::tuple<float, double>{};

    "M=N is a pass-through"_test = [] {
        KeepMInN<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.m_samples = 4U;
        block.n_samples = 4U;
        block.settingsChanged({}, {});

        const std::vector<float> in{1.f, 2.f, 3.f, 4.f};
        std::vector<float>       out(4);
        std::ignore = block.processBulk(std::span<const float>{in}, std::span<float>{out});

        for (std::size_t i = 0; i < 4; ++i) {
            expect(eq(out[i], in[i])) << "pass-through at i=" << i;
        }
    };

    "M=1 keeps only first sample"_test = [] {
        KeepMInN<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.m_samples = 1U;
        block.n_samples = 8U;
        block.settingsChanged({}, {});

        const std::vector<double> in{10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0};
        std::vector<double>       out(1);
        std::ignore = block.processBulk(std::span<const double>{in}, std::span<double>{out});

        expect(eq(out[0], 10.0)) << "keeps only first sample";
    };
};
