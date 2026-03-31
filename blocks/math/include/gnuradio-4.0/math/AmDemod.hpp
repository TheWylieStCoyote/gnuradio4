#ifndef GNURADIO_AM_DEMOD_HPP
#define GNURADIO_AM_DEMOD_HPP

#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::AmDemod, [T], [ std::complex<float>, std::complex<double> ])

template<typename T>
requires gr::meta::complex_like<T>
struct AmDemod : gr::Block<AmDemod<T>> {
    using Description = Doc<R""(
@brief AM (amplitude modulation) demodulator: recovers the envelope of a complex baseband signal.

Computes `out[n] = |in[n]|`, i.e. the instantaneous amplitude of the complex baseband
input. Equivalent to envelope detection. For DSB-SC or SSB demodulation, apply a
`DCBlocker` after this block to remove the carrier offset.
)"">;

    using value_type = typename T::value_type;

    PortIn<T>           in;
    PortOut<value_type> out;

    GR_MAKE_REFLECTABLE(AmDemod, in, out);

    [[nodiscard]] constexpr value_type processOne(T input) const noexcept { return std::abs(input); }
};

} // namespace gr::blocks::math

#endif // GNURADIO_AM_DEMOD_HPP
