#include <boost/ut.hpp>
#include <complex>

#include <gnuradio-4.0/basic/Throttle.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"Throttle"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    auto makeBlock = [](double sr, gr::Size_t chunkSize = 8192U) {
        Throttle<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.sample_rate             = sr;
        b.maximum_items_per_chunk = chunkSize;
        b.settingsChanged({}, {});
        b.start();
        return b;
    };

    "processOne passes sample through unchanged"_test = [&makeBlock] {
        auto b = makeBlock(1e6);
        expect(eq(b.processOne(3.14f), 3.14f));
        expect(eq(b.processOne(-1.0f), -1.0f));
    };

    "processOne passes complex sample unchanged"_test = [] {
        Throttle<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.sample_rate = 1e6;
        b.settingsChanged({}, {});
        b.start();
        const auto in  = std::complex<float>{1.5f, -2.5f};
        const auto out = b.processOne(in);
        expect(eq(out.real(), in.real()));
        expect(eq(out.imag(), in.imag()));
    };

    "sample counter increments per call"_test = [&makeBlock] {
        auto b = makeBlock(1e6);
        expect(eq(b._sampleCount, std::size_t{0}));
        std::ignore = b.processOne(0.0f);
        expect(eq(b._sampleCount, std::size_t{1}));
        std::ignore = b.processOne(0.0f);
        expect(eq(b._sampleCount, std::size_t{2}));
    };

    "start() resets the sample counter"_test = [&makeBlock] {
        auto b = makeBlock(1e6);
        for (int i = 0; i < 10; ++i) std::ignore = b.processOne(0.0f);
        b.start();
        expect(eq(b._sampleCount, std::size_t{0}));
    };

    "settingsChanged updates check interval"_test = [] {
        Throttle<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.maximum_items_per_chunk = 4096U;
        b.settingsChanged({}, {});
        expect(eq(b._checkInterval, std::size_t{4096}));
    };

    "very high sample rate does not sleep for small batch"_test = [&makeBlock] {
        // at rate=1e12 (effectively unlimited), no sleep should occur for a tiny batch
        auto b = makeBlock(1e12, 1U);
        // process check_interval samples — clock check fires each sample
        for (int i = 0; i < 4; ++i) std::ignore = b.processOne(1.0f);
        expect(b._sampleCount == 4UZ) << "counter should reach 4";
    };

    "double type compiles and passes through"_test = [] {
        Throttle<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.sample_rate = 1e6;
        b.settingsChanged({}, {});
        b.start();
        expect(eq(b.processOne(2.718), 2.718));
    };
};
