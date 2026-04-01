#ifndef GNURADIO_UDP_SOURCE_HPP
#define GNURADIO_UDP_SOURCE_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <bit>
#include <cerrno>
#include <complex>
#include <cstring>
#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::UdpSource, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct UdpSource : gr::Block<UdpSource<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief UDP datagram source — receives raw sample data over a UDP socket.

Binds to `bind_address`:`port` on `start()`. Each `processBulk` call blocks
until one UDP datagram arrives (or `timeout_ms` elapses) and copies the raw
bytes into the output span as type T values. `output_chunk_size` equals
`payload_size / sizeof(T)`. When `eof_on_disconnect` is true an empty receive
triggers `requestStop()`.
)"">;

    PortOut<T> out;

    Annotated<std::string, "bind_address",     Doc<"local bind address (empty = any)">>           bind_address{""};
    Annotated<std::uint16_t, "port",           Doc<"UDP port to bind">,                  Visible> port{50000U};
    Annotated<gr::Size_t,  "payload_size",     Doc<"expected bytes per UDP datagram">,   Visible> payload_size{1472U};
    Annotated<int,         "timeout_ms",       Doc<"receive timeout in milliseconds (0 = block)">>  timeout_ms{100};
    Annotated<bool,        "eof_on_disconnect", Doc<"stop block when receive returns 0 bytes">>    eof_on_disconnect{true};

    GR_MAKE_REFLECTABLE(UdpSource, out, bind_address, port, payload_size, timeout_ms, eof_on_disconnect);

    int         _sock{-1};
    std::size_t _samplesPerPacket{1};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _samplesPerPacket = std::max(std::size_t{1},
                                     static_cast<std::size_t>(payload_size) / sizeof(T));
        this->input_chunk_size  = static_cast<gr::Size_t>(1);
        this->output_chunk_size = static_cast<gr::Size_t>(_samplesPerPacket);
    }

    std::expected<void, gr::Error> start() {
        _sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (_sock < 0) {
            return std::unexpected(gr::Error{std::format("UdpSource: socket() failed: {}",
                                                          std::strerror(errno))});
        }

        const int reuseVal = 1;
        ::setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuseVal), sizeof(reuseVal));

        if (static_cast<int>(timeout_ms) > 0) {
            struct timeval tv{};
            tv.tv_sec  = static_cast<time_t>(timeout_ms) / 1000;
            tv.tv_usec = static_cast<suseconds_t>((static_cast<int>(timeout_ms) % 1000) * 1000);
            ::setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO,
                         reinterpret_cast<const char*>(&tv), sizeof(tv));
        }

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(static_cast<std::uint16_t>(port));
        const auto& addrStr = static_cast<const std::string&>(bind_address);
        if (addrStr.empty()) {
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            if (::inet_pton(AF_INET, addrStr.c_str(), &addr.sin_addr) <= 0) {
                ::close(_sock);
                _sock = -1;
                return std::unexpected(gr::Error{std::format("UdpSource: invalid address '{}'", addrStr)});
            }
        }

        if (::bind(_sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            const std::string err = std::strerror(errno);
            ::close(_sock);
            _sock = -1;
            return std::unexpected(gr::Error{std::format("UdpSource: bind() failed: {}", err)});
        }
        return {};
    }

    std::expected<void, gr::Error> stop() {
        if (_sock >= 0) {
            ::close(_sock);
            _sock = -1;
        }
        return {};
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<T> outSpan) noexcept {
        if (_sock < 0) return gr::work::Status::ERROR;

        const std::size_t payloadBytes = _samplesPerPacket * sizeof(T);
        std::vector<std::byte> buf(payloadBytes);

        const ::ssize_t n = ::recv(_sock, buf.data(), payloadBytes, 0);
        if (n <= 0) {
            if (static_cast<bool>(eof_on_disconnect)) this->requestStop();
            return gr::work::Status::OK;
        }

        const std::size_t nSamples = std::min(_samplesPerPacket,
                                               static_cast<std::size_t>(n) / sizeof(T));
        std::memcpy(outSpan.data(), buf.data(), nSamples * sizeof(T));
        if (nSamples < _samplesPerPacket) {
            std::fill(outSpan.begin() + static_cast<std::ptrdiff_t>(nSamples), outSpan.end(), T{});
        }
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_UDP_SOURCE_HPP
