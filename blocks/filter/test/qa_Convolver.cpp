#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <vector>

#include <gnuradio-4.0/filter/Convolver.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Convolver"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::filter;

    auto makeBlock = []<typename T>(std::size_t blockSz, std::vector<typename Convolver<T>::value_type> h) {
        Convolver<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.block_size = static_cast<gr::Size_t>(blockSz);
        block.kernel     = std::move(h);
        block.settingsChanged({}, {});
        return block;
    };

    "identity kernel passes signal unchanged"_test = [&makeBlock]<typename T> {
        auto block = makeBlock.template operator()<T>(8UZ, {typename Convolver<T>::value_type{1}});

        const std::size_t L = 8;
        std::vector<T>    in(L);
        std::vector<T>    out(L);
        for (std::size_t i = 0; i < L; ++i) in[i] = static_cast<T>(static_cast<double>(i + 1));
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        for (std::size_t i = 0; i < L; ++i) {
            expect(approx(static_cast<double>(std::real(out[i])), static_cast<double>(std::real(in[i])), 1e-4))
                << "identity at i=" << i;
        }
    } | std::tuple<float, double>{};

    "3-tap FIR kernel produces correct convolution"_test = [&makeBlock] {
        // h = [0.5, 0.25, 0.25], impulse input → output is h itself
        using T            = float;
        using VT           = Convolver<T>::value_type;
        const std::size_t L = 8;
        auto              block = makeBlock.template operator()<T>(L, {VT{0.5}, VT{0.25}, VT{0.25}});

        std::vector<T> in(L, 0.f);
        in[0] = 1.f; // unit impulse
        std::vector<T> out(L);
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        expect(approx(static_cast<double>(out[0]), 0.5,  1e-4)) << "h[0]";
        expect(approx(static_cast<double>(out[1]), 0.25, 1e-4)) << "h[1]";
        expect(approx(static_cast<double>(out[2]), 0.25, 1e-4)) << "h[2]";
        expect(approx(static_cast<double>(out[3]), 0.0,  1e-4)) << "h[3]";
    };

    "overlap is preserved across block boundaries"_test = [&makeBlock] {
        // h = [0.5, 0.5], block_size = 4
        // Send impulse in first block; tail should appear in second block
        using T            = double;
        using VT           = Convolver<T>::value_type;
        const std::size_t L = 4;
        auto              block = makeBlock.template operator()<T>(L, {VT{0.5}, VT{0.5}});

        std::vector<T> in1(L, 0.0), out1(L), in2(L, 0.0), out2(L);
        in1[L - 1] = 1.0; // impulse at last sample of block 1

        std::ignore = block.processBulk(std::span<const T>{in1}, std::span<T>{out1});
        std::ignore = block.processBulk(std::span<const T>{in2}, std::span<T>{out2});

        // h[0] * impulse appears in out1[L-1]
        expect(approx(out1[L - 1], 0.5, 1e-6)) << "first tap in block 1";
        // h[1] * impulse carried into block 2
        expect(approx(out2[0], 0.5, 1e-6)) << "second tap in block 2 via overlap";
    };

    "settingsChanged reinitialises overlap buffer"_test = [&makeBlock] {
        using T  = float;
        using VT = Convolver<T>::value_type;
        auto block = makeBlock.template operator()<T>(8UZ, {VT{1}, VT{0.5}});

        // dirty overlap with an impulse
        std::vector<T> in(8, 0.f), out(8);
        in[7] = 1.f;
        std::ignore = block.processBulk(std::span<const T>{in}, std::span<T>{out});

        block.settingsChanged({}, {});
        for (const auto& v : block._overlapBuf) {
            expect(approx(static_cast<double>(v.real()), 0.0, 1e-9) &&
                   approx(static_cast<double>(v.imag()), 0.0, 1e-9)) << "overlap reset";
        }
    };
};
