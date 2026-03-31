#ifndef GNURADIO_CRC_COMPUTE_HPP
#define GNURADIO_CRC_COMPUTE_HPP

#include <cstdint>
#include <span>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::CrcCompute, , )

struct CrcCompute : gr::Block<CrcCompute, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief CRC-8 appender for fixed-length packets.

For every `packet_len`-byte input packet the block outputs `packet_len + 1` bytes:
the original data followed by a CRC-8 byte (polynomial 0x07, initial value 0x00).
The chunk sizes are configured in `settingsChanged`.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    Annotated<gr::Size_t, "packet_len", Doc<"bytes per packet (excluding CRC)">, Visible> packet_len{16U};

    GR_MAKE_REFLECTABLE(CrcCompute, in, out, packet_len);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t L     = static_cast<std::size_t>(packet_len);
        this->input_chunk_size  = L;
        this->output_chunk_size = L + 1UZ;
    }

    // CRC-8 polynomial 0x07 (ITU-T / DVB)
    static constexpr uint8_t computeCrc(std::span<const uint8_t> data) noexcept {
        uint8_t crc = 0;
        for (const uint8_t byte : data) {
            crc ^= byte;
            for (int i = 0; i < 8; ++i) {
                const uint32_t shifted = static_cast<uint32_t>(crc) << 1U;
                crc = static_cast<uint8_t>((crc & 0x80U) != 0U ? (shifted ^ 0x07U) : shifted);
            }
        }
        return crc;
    }

    [[nodiscard]] work::Status processBulk(std::span<const uint8_t> inSpan, std::span<uint8_t> outSpan) noexcept {
        const std::size_t L = static_cast<std::size_t>(packet_len);
        std::copy(inSpan.begin(), inSpan.end(), outSpan.begin());
        outSpan[L] = computeCrc(inSpan);
        return work::Status::OK;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_CRC_COMPUTE_HPP
