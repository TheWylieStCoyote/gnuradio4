#include <boost/ut.hpp>
#include <complex>
#include <tuple>
#include <vector>

#include <gnuradio-4.0/basic/HeaderPayloadDemux.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"HeaderPayloadDemux"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::basic;

    auto makeBlock = [](gr::Size_t hLen, gr::Size_t pLen) {
        HeaderPayloadDemux<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.header_length  = hLen;
        b.payload_length = pLen;
        b.settingsChanged({}, {});
        return b;
    };

    "idle state outputs zeros for both ports"_test = [&makeBlock] {
        auto b = makeBlock(4U, 8U);
        const auto [h, p] = b.processOne(3.0f);
        expect(eq(h, 0.0f)) << "header is zero in idle";
        expect(eq(p, 0.0f)) << "payload is zero in idle";
    };

    "header state routes sample to header port"_test = [&makeBlock] {
        auto b = makeBlock(4U, 8U);
        b._state = HeaderPayloadDemux<float>::State::Header;
        b._count = 0UZ;

        const auto [h, p] = b.processOne(1.0f);
        expect(eq(h, 1.0f)) << "sample goes to header";
        expect(eq(p, 0.0f)) << "payload is zero";
    };

    "payload state routes sample to payload port"_test = [&makeBlock] {
        auto b = makeBlock(4U, 8U);
        b._state = HeaderPayloadDemux<float>::State::Payload;
        b._count = 0UZ;

        const auto [h, p] = b.processOne(2.0f);
        expect(eq(h, 0.0f)) << "header is zero";
        expect(eq(p, 2.0f)) << "sample goes to payload";
    };

    "header state transitions to payload after header_length samples"_test = [&makeBlock] {
        auto b = makeBlock(3U, 8U);
        b._state = HeaderPayloadDemux<float>::State::Header;
        b._count = 0UZ;

        std::ignore = b.processOne(1.0f); // count=1
        std::ignore = b.processOne(1.0f); // count=2
        std::ignore = b.processOne(1.0f); // count=3 → transition

        expect(b._state == HeaderPayloadDemux<float>::State::Payload) << "state is Payload";
        expect(eq(b._count, std::size_t{0})) << "count reset";
    };

    "payload state transitions to idle after payload_length samples"_test = [&makeBlock] {
        auto b = makeBlock(4U, 3U);
        b._state = HeaderPayloadDemux<float>::State::Payload;
        b._count = 0UZ;

        std::ignore = b.processOne(1.0f); // count=1
        std::ignore = b.processOne(1.0f); // count=2
        std::ignore = b.processOne(1.0f); // count=3 → transition

        expect(b._state == HeaderPayloadDemux<float>::State::Idle) << "state is Idle";
        expect(eq(b._count, std::size_t{0})) << "count reset";
    };

    "last header sample is still routed to header"_test = [&makeBlock] {
        auto b = makeBlock(2U, 8U);
        b._state = HeaderPayloadDemux<float>::State::Header;
        b._count = 0UZ;

        std::ignore = b.processOne(5.0f); // count=1, stays Header
        const auto [h, p] = b.processOne(7.0f); // count=2, transitions out

        expect(eq(h, 7.0f)) << "last header sample still goes to header port";
        expect(eq(p, 0.0f));
    };

    "last payload sample is still routed to payload"_test = [&makeBlock] {
        auto b = makeBlock(4U, 2U);
        b._state = HeaderPayloadDemux<float>::State::Payload;
        b._count = 0UZ;

        std::ignore = b.processOne(3.0f); // count=1, stays Payload
        const auto [h, p] = b.processOne(9.0f); // count=2, transitions out

        expect(eq(h, 0.0f));
        expect(eq(p, 9.0f)) << "last payload sample still goes to payload port";
    };

    "settingsChanged resets to idle"_test = [&makeBlock] {
        auto b = makeBlock(4U, 8U);
        b._state = HeaderPayloadDemux<float>::State::Payload;
        b._count = 5UZ;

        b.settingsChanged({}, {});

        expect(b._state == HeaderPayloadDemux<float>::State::Idle) << "reset to idle";
        expect(eq(b._count, std::size_t{0})) << "count reset";
    };

    "full header+payload sequence from manually armed state"_test = [] {
        HeaderPayloadDemux<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.header_length  = 4U;
        b.payload_length = 4U;
        b.settingsChanged({}, {});

        // arm manually (simulates trigger tag arriving)
        b._state = HeaderPayloadDemux<float>::State::Header;
        b._count = 0UZ;

        std::vector<float> hSamples, pSamples;
        for (float v = 1.0f; v <= 8.0f; v += 1.0f) {
            const auto [h, p] = b.processOne(v);
            hSamples.push_back(h);
            pSamples.push_back(p);
        }

        // first 4 go to header, next 4 go to payload
        expect(eq(hSamples[0], 1.0f));
        expect(eq(hSamples[3], 4.0f));
        expect(eq(hSamples[4], 0.0f));
        expect(eq(pSamples[3], 0.0f));
        expect(eq(pSamples[4], 5.0f));
        expect(eq(pSamples[7], 8.0f));
        expect(b._state == HeaderPayloadDemux<float>::State::Idle) << "returns to idle";
    };

    "complex<float> compiles and routes correctly"_test = [] {
        HeaderPayloadDemux<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.header_length  = 2U;
        b.payload_length = 2U;
        b.settingsChanged({}, {});

        b._state = HeaderPayloadDemux<std::complex<float>>::State::Header;
        b._count = 0UZ;

        const auto [h, p] = b.processOne({1.0f, 2.0f});
        expect(eq(h.real(), 1.0f));
        expect(eq(p.real(), 0.0f));
    };
};
