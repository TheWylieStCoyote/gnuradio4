#ifndef GNURADIO_PACK_BITS_HPP
#define GNURADIO_PACK_BITS_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::PackBits, , )

struct PackBits : gr::Block<PackBits, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Pack individual bit bytes into multi-bit words.

Reads `bits_per_chunk` input bytes (each carrying one bit in its LSB) and packs
them into a single output byte, MSB first. `bits_per_chunk` must be in [1, 8].
Input and output chunk sizes are set accordingly.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    Annotated<gr::Size_t, "bits_per_chunk", Doc<"number of input bits per output byte [1-8]">, Visible> bits_per_chunk{8U};

    GR_MAKE_REFLECTABLE(PackBits, in, out, bits_per_chunk);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t bpc    = std::clamp(static_cast<std::size_t>(bits_per_chunk), std::size_t{1}, std::size_t{8});
        this->input_chunk_size  = bpc;
        this->output_chunk_size = 1UZ;
    }

    [[nodiscard]] work::Status processBulk(std::span<const uint8_t> inSpan, std::span<uint8_t> outSpan) noexcept {
        const std::size_t bpc = std::clamp(static_cast<std::size_t>(bits_per_chunk), std::size_t{1}, std::size_t{8});
        const std::size_t nChunks = inSpan.size() / bpc;
        for (std::size_t i = 0; i < nChunks; ++i) {
            uint8_t packed = 0;
            for (std::size_t b = 0; b < bpc; ++b) {
                const auto shifted = static_cast<uint8_t>(packed << 1U);
                const auto bit     = static_cast<uint8_t>(inSpan[i * bpc + b] & 0x01U);
                packed             = static_cast<uint8_t>(shifted | bit);
            }
            outSpan[i] = packed;
        }
        return work::Status::OK;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_PACK_BITS_HPP
