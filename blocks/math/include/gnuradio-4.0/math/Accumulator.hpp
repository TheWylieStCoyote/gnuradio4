#ifndef GNURADIO_ACCUMULATOR_HPP
#define GNURADIO_ACCUMULATOR_HPP

#include <complex>
#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Accumulator, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Accumulator : gr::Block<Accumulator<T>> {
    using Description = Doc<R""(
@brief Running sum (numerical integration) of the input stream.

Computes `out[n] = out[n-1] + in[n]`, accumulating all samples since the last
reset.  If `reset_tag_key` is non-empty, the accumulator resets to zero whenever
a tag with that key arrives on the input.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<std::string, "reset_tag_key", Doc<"tag key that resets the accumulator to zero (empty = never reset)">> reset_tag_key{};

    GR_MAKE_REFLECTABLE(Accumulator, in, out, reset_tag_key);

    T _sum{};

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        const auto& tag = this->mergedInputTag();
        if (!reset_tag_key->empty() && tag.map.contains(static_cast<const std::string&>(reset_tag_key))) {
            _sum = T{};
        }
        _sum += input;
        return _sum;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_ACCUMULATOR_HPP
