#include <boost/ut.hpp>
#include <cmath>

#include <gnuradio-4.0/math/DbConvert.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"PowerToDb"> powerToDbTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    "0 dB when input equals reference"_test = []<typename T> {
        PowerToDb<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        // ref defaults to 1; 10*log10(1/1) = 0
        expect(approx(static_cast<double>(block.processOne(T{1})), 0.0, 1e-5)) << "0 dB";
    } | kTypes;

    "10 dB for 10x power ratio"_test = []<typename T> {
        PowerToDb<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        expect(approx(static_cast<double>(block.processOne(T{10})), 10.0, 1e-5)) << "10x power";
    } | kTypes;

    "amplitude mode: 20 dB for 10x amplitude ratio"_test = []<typename T> {
        PowerToDb<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.amplitude_mode = true;
        expect(approx(static_cast<double>(block.processOne(T{10})), 20.0, 1e-5)) << "20 log10";
    } | kTypes;

    "custom reference shifts output"_test = []<typename T> {
        PowerToDb<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.ref = T{2};
        // 10*log10(2/2) = 0
        expect(approx(static_cast<double>(block.processOne(T{2})), 0.0, 1e-5)) << "ref=2, in=2";
        // 10*log10(20/2) = 10
        expect(approx(static_cast<double>(block.processOne(T{20})), 10.0, 1e-5)) << "ref=2, in=20";
    } | kTypes;
};

const boost::ut::suite<"DbToPower"> dbToPowerTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::math;

    constexpr auto kTypes = std::tuple<float, double>{};

    "0 dB produces output equal to reference"_test = []<typename T> {
        DbToPower<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        expect(approx(static_cast<double>(block.processOne(T{0})), 1.0, 1e-5)) << "0 dB → 1";
    } | kTypes;

    "10 dB produces 10x reference"_test = []<typename T> {
        DbToPower<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        expect(approx(static_cast<double>(block.processOne(T{10})), 10.0, 1e-4)) << "10 dB → 10";
    } | kTypes;

    "amplitude mode: 20 dB produces 10x reference"_test = []<typename T> {
        DbToPower<T> block({});
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.amplitude_mode = true;
        expect(approx(static_cast<double>(block.processOne(T{20})), 10.0, 1e-4)) << "20 dB ampl → 10";
    } | kTypes;

    "PowerToDb and DbToPower are inverses"_test = []<typename T> {
        PowerToDb<T> toDb({});
        toDb.settings().init();
        std::ignore = toDb.settings().applyStagedParameters();
        DbToPower<T> fromDb({});
        fromDb.settings().init();
        std::ignore = fromDb.settings().applyStagedParameters();

        const T orig   = T{42};
        const T result = fromDb.processOne(toDb.processOne(orig));
        expect(approx(static_cast<double>(result), static_cast<double>(orig), 1e-4)) << "round trip";
    } | kTypes;
};
