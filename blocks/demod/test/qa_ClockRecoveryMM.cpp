#include <boost/ut.hpp>
#include <complex>
#include <vector>

#include <gnuradio-4.0/demod/ClockRecoveryMM.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"ClockRecoveryMM"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::demod;

    "settingsChanged initialises omega to sps"_test = []<typename T> {
        ClockRecoveryMM<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.sps = static_cast<T>(4);
        block.settingsChanged({}, {});

        expect(approx(static_cast<double>(block._omega), 4.0, 0.01)) << "omega starts at sps";
        expect(approx(static_cast<double>(block._mu), 0.5, 0.01))    << "mu starts at 0.5";
    } | std::tuple<float, double>{};

    "omega mid and limit are derived from sps"_test = [] {
        ClockRecoveryMM<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.sps              = 8.f;
        block.omega_rel_limit  = 0.01f;
        block.settingsChanged({}, {});

        expect(approx(static_cast<double>(block._omegaMid), 8.0, 0.01));
        expect(approx(static_cast<double>(block._omegaLimit), 0.08, 0.01));
    };
};
