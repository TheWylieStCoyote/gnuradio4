#ifndef GNURADIO_DIFFERENTIAL_DECODER_HPP
#define GNURADIO_DIFFERENTIAL_DECODER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::DifferentialDecoder, , )

struct DifferentialDecoder : gr::Block<DifferentialDecoder> {
    using Description = Doc<R""(
@brief Differential binary decoder.

Decodes a differentially encoded bit stream: `out[n] = in[n] XOR in[n-1]`. The initial
reference bit `_prev` is zero at reset. Each byte represents one bit (LSB). Inverse of
`DifferentialEncoder`.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    GR_MAKE_REFLECTABLE(DifferentialDecoder, in, out);

    uint8_t _prev{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _prev = 0;
    }

    [[nodiscard]] uint8_t processOne(uint8_t bit) noexcept {
        const uint8_t decoded = static_cast<uint8_t>((bit ^ _prev) & 0x01U);
        _prev                 = static_cast<uint8_t>(bit & 0x01U);
        return decoded;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_DIFFERENTIAL_DECODER_HPP
