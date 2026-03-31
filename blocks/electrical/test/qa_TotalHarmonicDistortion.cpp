#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/DataSet.hpp>
#include <gnuradio-4.0/electrical/TotalHarmonicDistortion.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"TotalHarmonicDistortion"> tests = [] {
    using namespace boost::ut;
    using namespace gr::electrical;

    "pure fundamental has zero THD"_test = []<typename T> {
        TotalHarmonicDistortion<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // DataSet with [V1=1, V2=0, V3=0, phase1, phase2, phase3]
        gr::DataSet<T> ds{};
        ds.signal_values = {T{1}, T{}, T{}, T{}, T{}, T{}};

        const T thd = block.processOne(ds);
        expect(approx(static_cast<double>(thd), 0.0, 1e-6)) << "pure fundamental → zero THD";
    } | std::tuple<float, double>{};

    "known harmonic content gives correct THD"_test = [] {
        TotalHarmonicDistortion<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        // V1=1, V2=0.1, V3=0.1 → THD = sqrt(0.01+0.01)/1 = sqrt(0.02) ≈ 0.1414
        gr::DataSet<float> ds{};
        ds.signal_values = {1.f, 0.1f, 0.1f, 0.f, 0.f, 0.f};

        const float thd    = block.processOne(ds);
        const float expect_thd = std::sqrt(0.02f);
        expect(approx(static_cast<double>(thd), static_cast<double>(expect_thd), 1e-5)) << "THD matches sqrt(V2²+V3²)/V1";
    };

    "zero fundamental gives zero output"_test = [] {
        TotalHarmonicDistortion<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();

        gr::DataSet<float> ds{};
        ds.signal_values = {0.f, 0.1f, 0.f, 0.f, 0.f, 0.f};

        const float thd = block.processOne(ds);
        expect(eq(thd, 0.f));
    };
};
