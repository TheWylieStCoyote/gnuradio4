#ifndef GNURADIO_UNPACK_BITS_HPP
#define GNURADIO_UNPACK_BITS_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::UnpackBits, , )

struct UnpackBits : gr::Block<UnpackBits, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Unpack multi-bit bytes into individual bit bytes.

Reads one input byte and unpacks `bits_per_chunk` bits from it (MSB first), writing
each as a single byte (value 0 or 1) to the output. Inverse of `PackBits`.
`bits_per_chunk` must be in [1, 8].
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    Annotated<gr::Size_t, "bits_per_chunk", Doc<"number of bits to unpack per input byte [1-8]">, Visible> bits_per_chunk{8U};

    GR_MAKE_REFLECTABLE(UnpackBits, in, out, bits_per_chunk);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t bpc    = std::clamp(static_cast<std::size_t>(bits_per_chunk), std::size_t{1}, std::size_t{8});
        this->input_chunk_size  = 1UZ;
        this->output_chunk_size = bpc;
    }

    [[nodiscard]] work::Status processBulk(std::span<const uint8_t> inSpan, std::span<uint8_t> outSpan) noexcept {
        const std::size_t bpc = std::clamp(static_cast<std::size_t>(bits_per_chunk), std::size_t{1}, std::size_t{8});
        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            const uint8_t byte = inSpan[i];
            for (std::size_t b = 0; b < bpc; ++b) {
                const std::size_t shift = bpc - 1UZ - b;
                outSpan[i * bpc + b]   = static_cast<uint8_t>((byte >> shift) & 0x01U);
            }
        }
        return work::Status::OK;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_UNPACK_BITS_HPP
