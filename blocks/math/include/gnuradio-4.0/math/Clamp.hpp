#ifndef GNURADIO_CLAMP_HPP
#define GNURADIO_CLAMP_HPP

#include <algorithm>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Clamp, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Clamp : gr::Block<Clamp<T>> {
    using Description = Doc<R""(
@brief Clips each sample to the range [min, max].

For real types, applies `std::clamp(x, min, max)`. For complex types, clamps
the real and imaginary parts independently. When the input is complex, `min` and
`max` refer to `value_type` (the underlying real scalar).
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<value_type, "min", Doc<"lower clip bound (inclusive)">, Visible> min{value_type{-1}};
    Annotated<value_type, "max", Doc<"upper clip bound (inclusive)">, Visible> max{value_type{1}};

    GR_MAKE_REFLECTABLE(Clamp, in, out, min, max);

    [[nodiscard]] constexpr T processOne(T input) const noexcept {
        if constexpr (gr::meta::complex_like<T>) {
            return {std::clamp(input.real(), static_cast<value_type>(min), static_cast<value_type>(max)),
                    std::clamp(input.imag(), static_cast<value_type>(min), static_cast<value_type>(max))};
        } else {
            return std::clamp(input, static_cast<T>(min), static_cast<T>(max));
        }
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_CLAMP_HPP
