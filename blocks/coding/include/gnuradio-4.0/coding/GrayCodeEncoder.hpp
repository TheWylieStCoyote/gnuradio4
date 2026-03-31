#ifndef GNURADIO_GRAY_CODE_ENCODER_HPP
#define GNURADIO_GRAY_CODE_ENCODER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::GrayCodeEncoder, , )

struct GrayCodeEncoder : gr::Block<GrayCodeEncoder> {
    using Description = Doc<R""(
@brief Binary to Gray code encoder.

Converts a binary integer to its Gray code representation: `out = n XOR (n >> 1)`.
Adjacent Gray code values differ in exactly one bit, which minimises bit errors in
digital position sensors, ADCs, and QAM symbol mapping.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    GR_MAKE_REFLECTABLE(GrayCodeEncoder, in, out);

    [[nodiscard]] constexpr uint8_t processOne(uint8_t n) const noexcept {
        return static_cast<uint8_t>(n ^ (n >> 1U));
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_GRAY_CODE_ENCODER_HPP
