#ifndef GNURADIO_SCRAMBLER_HPP
#define GNURADIO_SCRAMBLER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::coding {

GR_REGISTER_BLOCK(gr::blocks::coding::Scrambler, , )

struct Scrambler : gr::Block<Scrambler> {
    using Description = Doc<R""(
@brief Additive LFSR scrambler/descrambler.

XORs each input byte (LSB) with one output bit of a linear feedback shift register.
The LFSR is defined by its `mask` (tap polynomial, excluding the feedback bit) and
initial `seed`. `len` is the register width in bits [1–32]. The same block
configuration acts as both scrambler and descrambler since XOR is self-inverse.
Useful for whitening bit streams before modulation.
)"">;

    PortIn<uint8_t>  in;
    PortOut<uint8_t> out;

    Annotated<uint32_t, "mask", Doc<"LFSR tap mask (XOR feedback polynomial)">, Visible> mask{0xB8U};  // x⁸+x⁶+x⁵+x⁴+1 (DVB-S)
    Annotated<uint32_t, "seed", Doc<"LFSR initial state (non-zero)">,            Visible> seed{0xFFU};
    Annotated<gr::Size_t, "len", Doc<"LFSR width in bits [1-32]">,              Visible> len{8U};

    GR_MAKE_REFLECTABLE(Scrambler, in, out, mask, seed, len);

    uint32_t _lfsr{0xFFU};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _lfsr = static_cast<uint32_t>(seed);
    }

    [[nodiscard]] uint8_t processOne(uint8_t bit) noexcept {
        const uint8_t lfsrBit = static_cast<uint8_t>(_lfsr & 0x01U);
        // feedback = XOR of all tapped positions
        const uint32_t feedback = static_cast<uint32_t>(__builtin_popcount(_lfsr & static_cast<uint32_t>(mask))) & 1U;
        const uint32_t width    = static_cast<uint32_t>(std::clamp(static_cast<std::size_t>(len), std::size_t{1}, std::size_t{32}));
        _lfsr = static_cast<uint32_t>((_lfsr >> 1U) | (feedback << (width - 1U)));
        return static_cast<uint8_t>((bit ^ lfsrBit) & 0x01U);
    }
};

} // namespace gr::blocks::coding

#endif // GNURADIO_SCRAMBLER_HPP
