#include <boost/ut.hpp>
#include <complex>

#include <gnuradio-4.0/fileio/UdpSink.hpp>
#include <gnuradio-4.0/fileio/UdpSource.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"UdpSource"> sourceTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "settingsChanged computes _samplesPerPacket for float"_test = [] {
        UdpSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.payload_size = gr::Size_t{16U}; // 4 floats
        b.settingsChanged({}, {});
        expect(eq(b._samplesPerPacket, std::size_t{4}));
    };

    "settingsChanged with complex<float> divides by sizeof(complex<float>)"_test = [] {
        UdpSource<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.payload_size = gr::Size_t{32U}; // 4 complex<float>
        b.settingsChanged({}, {});
        expect(eq(b._samplesPerPacket, std::size_t{4}));
    };

    "start() binds to local port and stop() closes socket"_test = [] {
        UdpSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.port       = 0U; // let OS pick a free port
        b.timeout_ms = 10;
        b.settingsChanged({}, {});

        const auto startResult = b.start();
        expect(startResult.has_value()) << "start() should bind successfully";
        expect(b._sock >= 0) << "socket descriptor should be valid after start()";

        const auto stopResult = b.stop();
        expect(stopResult.has_value()) << "stop() should succeed";
        expect(eq(b._sock, -1)) << "socket should be closed after stop()";
    };

    "start() fails for invalid bind address"_test = [] {
        UdpSource<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.bind_address = "999.999.999.999"; // invalid
        b.settingsChanged({}, {});
        const auto result = b.start();
        expect(!result.has_value()) << "start() should fail for invalid IP";
    };

    "double type compiles and _samplesPerPacket is correct"_test = [] {
        UdpSource<double> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.payload_size = gr::Size_t{16U}; // 2 doubles
        b.settingsChanged({}, {});
        expect(eq(b._samplesPerPacket, std::size_t{2}));
    };
};

const boost::ut::suite<"UdpSink"> sinkTests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fileio;

    "settingsChanged computes _samplesPerPacket for float"_test = [] {
        UdpSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.payload_size = gr::Size_t{20U}; // 5 floats
        b.settingsChanged({}, {});
        expect(eq(b._samplesPerPacket, std::size_t{5}));
    };

    "start() creates socket and stop() closes it"_test = [] {
        UdpSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.address      = "127.0.0.1";
        b.port         = 50002U;
        b.payload_size = gr::Size_t{16U};
        b.settingsChanged({}, {});

        const auto startResult = b.start();
        expect(startResult.has_value()) << "start() should succeed";
        expect(b._sock >= 0) << "socket should be valid";

        const auto stopResult = b.stop();
        expect(stopResult.has_value()) << "stop() should succeed";
        expect(eq(b._sock, -1)) << "socket should be closed";
    };

    "start() fails for invalid destination address"_test = [] {
        UdpSink<float> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.address = "not-an-ip";
        b.settingsChanged({}, {});
        const auto result = b.start();
        expect(!result.has_value()) << "start() should fail for invalid address";
    };

    "complex<float> compiles and _samplesPerPacket is correct"_test = [] {
        UdpSink<std::complex<float>> b{};
        b.settings().init();
        std::ignore = b.settings().applyStagedParameters();
        b.payload_size = gr::Size_t{32U}; // 4 complex<float>
        b.settingsChanged({}, {});
        expect(eq(b._samplesPerPacket, std::size_t{4}));
    };
};
