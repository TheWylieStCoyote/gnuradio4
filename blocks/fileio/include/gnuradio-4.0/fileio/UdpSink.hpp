#ifndef GNURADIO_UDP_SINK_HPP
#define GNURADIO_UDP_SINK_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <complex>
#include <cstring>
#include <string>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fileio {

GR_REGISTER_BLOCK(gr::blocks::fileio::UdpSink, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct UdpSink : gr::Block<UdpSink<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief UDP datagram sink — sends a sample stream as UDP datagrams.

Packs `payload_size / sizeof(T)` samples into each datagram and sends it to
`address`:`port`. The socket is opened on `start()` and closed on `stop()`.
`input_chunk_size` is set to match `payload_size / sizeof(T)` so the scheduler
provides exactly one packet's worth of samples per call.
)"">;

    PortIn<T> in;

    Annotated<std::string,   "address",      Doc<"destination IP address">,                         Visible> address{"127.0.0.1"};
    Annotated<std::uint16_t, "port",         Doc<"destination UDP port">,                           Visible> port{50001U};
    Annotated<gr::Size_t,    "payload_size", Doc<"bytes per UDP datagram (aligned to sizeof(T))">,  Visible> payload_size{1472U};

    GR_MAKE_REFLECTABLE(UdpSink, in, address, port, payload_size);

    int         _sock{-1};
    std::size_t _samplesPerPacket{1};
    struct sockaddr_in _destAddr{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _samplesPerPacket = std::max(std::size_t{1},
                                     static_cast<std::size_t>(payload_size) / sizeof(T));
        this->input_chunk_size  = static_cast<gr::Size_t>(_samplesPerPacket);
        this->output_chunk_size = static_cast<gr::Size_t>(1);
    }

    std::expected<void, gr::Error> start() {
        _sock = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (_sock < 0) {
            return std::unexpected(gr::Error{std::format("UdpSink: socket() failed: {}",
                                                          std::strerror(errno))});
        }

        std::memset(&_destAddr, 0, sizeof(_destAddr));
        _destAddr.sin_family = AF_INET;
        _destAddr.sin_port   = htons(static_cast<std::uint16_t>(port));
        const auto& addrStr  = static_cast<const std::string&>(address);
        if (::inet_pton(AF_INET, addrStr.c_str(), &_destAddr.sin_addr) <= 0) {
            ::close(_sock);
            _sock = -1;
            return std::unexpected(gr::Error{std::format("UdpSink: invalid address '{}'", addrStr)});
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

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan) noexcept {
        if (_sock < 0) return gr::work::Status::ERROR;

        const std::size_t n = std::min(_samplesPerPacket, inSpan.size());
        ::sendto(_sock,
                 inSpan.data(),
                 n * sizeof(T),
                 0,
                 reinterpret_cast<const struct sockaddr*>(&_destAddr),
                 sizeof(_destAddr));
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fileio

#endif // GNURADIO_UDP_SINK_HPP
