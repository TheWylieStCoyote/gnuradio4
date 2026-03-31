#ifndef GNURADIO_DIFFERENTIAL_ENCODER_HPP
#define GNURADIO_DIFFERENTIAL_ENCODER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::DifferentialEncoder, , )

struct DifferentialEncoder : gr::Block<DifferentialEncoder> {
    using Description = Doc<R""(
@brief Differential binary encoder.

Encodes a sequence of bits differentially: `out[n] = out[n-1] XOR in[n]`. The initial
reference bit `_prev` is zero at reset. Each byte represents one bit (LSB). Commonly
used before BPSK/DPSK modulation to eliminate phase ambiguity.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    GR_MAKE_REFLECTABLE(DifferentialEncoder, in, out);

    uint8_t _prev{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _prev = 0;
    }

    [[nodiscard]] uint8_t processOne(uint8_t bit) noexcept {
        _prev = static_cast<uint8_t>((_prev ^ bit) & 0x01U);
        return _prev;
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_DIFFERENTIAL_ENCODER_HPP
