#ifndef GNURADIO_GRAY_CODE_DECODER_HPP
#define GNURADIO_GRAY_CODE_DECODER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::GrayCodeDecoder, , )

struct GrayCodeDecoder : gr::Block<GrayCodeDecoder> {
    using Description = Doc<R""(
@brief Gray code to binary decoder.

Converts a Gray code value back to its binary integer representation via the
iterative XOR-fold: `mask = g >> 1; while (mask) { g ^= mask; mask >>= 1; }`.
Inverse of `GrayCodeEncoder`.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    GR_MAKE_REFLECTABLE(GrayCodeDecoder, in, out);

    [[nodiscard]] constexpr uint8_t processOne(uint8_t g) const noexcept {
        uint8_t mask = static_cast<uint8_t>(g >> 1U);
        while (mask != 0U) {
            g    = static_cast<uint8_t>(g ^ mask);
            mask = static_cast<uint8_t>(mask >> 1U);
        }
        return g;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_GRAY_CODE_DECODER_HPP
