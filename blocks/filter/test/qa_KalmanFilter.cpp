#include <boost/ut.hpp>
#include <cmath>
#include <vector>

#include <gnuradio-4.0/filter/KalmanFilter.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"KalmanFilter"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    auto makeBlock = []<typename T>(T a, T c, T q, T r, T p0 = T{1}) {
        KalmanFilter<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.A  = a;
        block.C  = c;
        block.Q  = q;
        block.R  = r;
        block.P0 = p0;
        block.settingsChanged({}, {});
        return block;
    };

    "covariance decreases after each update (noiseless observation)"_test = [&makeBlock]<typename T> {
        // Q=0, R=1 → P should decay each step
        auto block = makeBlock.template operator()<T>(T{1}, T{1}, T{0}, T{1}, T{1});
        T prevP = block._P;
        for (int i = 0; i < 10; ++i) {
            std::ignore = block.processOne(T{});
            expect(block._P <= prevP) << "P decreases or stays equal";
            prevP = block._P;
        }
    } | std::tuple<float, double>{};

    "tracks constant signal with low steady-state error"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(T{1}, T{1}, static_cast<T>(0.01), T{1}, T{1});

        T lastOut{};
        for (int i = 0; i < 200; ++i) {
            const T noise = static_cast<T>(0.1 * ((i % 5) - 2));
            lastOut = block.processOne(T{1} + noise);
        }
        expect(approx(static_cast<double>(lastOut), 1.0, 0.5)) << "tracks constant";
    } | std::tuple<float, double>{};

    "settingsChanged resets state and covariance"_test = [&makeBlock] {
        auto block = makeBlock.template operator()<double>(1.0, 1.0, 0.1, 1.0, 5.0);
        for (int i = 0; i < 20; ++i) std::ignore = block.processOne(static_cast<double>(i));

        block.settingsChanged({}, {});
        expect(eq(block._xHat, 0.0)) << "state reset";
        expect(approx(block._P, 5.0, 1e-9)) << "covariance reset to P0";
    };

    "single perfect observation (R→0) snaps to measurement"_test = [&makeBlock] {
        // Very small R → gain ≈ 1 → output ≈ measurement
        auto block = makeBlock.template operator()<double>(1.0, 1.0, 0.0, 1e-8, 1.0);
        const double out = block.processOne(42.0);
        expect(approx(out, 42.0, 0.01)) << "snaps to measurement when R≈0";
    };
};
