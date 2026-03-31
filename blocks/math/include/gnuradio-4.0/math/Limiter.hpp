#ifndef GNURADIO_LIMITER_HPP
#define GNURADIO_LIMITER_HPP

#include <algorithm>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Limiter, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Limiter : gr::Block<Limiter<T>> {
    using Description = Doc<R""(
@brief Symmetric hard amplitude limiter: clips |x| to `limit` while preserving direction.

For real types: `out = clamp(x, -limit, limit)`.
For complex types: if `|x| > limit`, scale x so `|out| = limit`; the phase is preserved.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<value_type, "limit", Doc<"maximum amplitude (must be > 0)">, Visible> limit{value_type{1}};

    GR_MAKE_REFLECTABLE(Limiter, in, out, limit);

    [[nodiscard]] constexpr T processOne(T input) const noexcept {
        if constexpr (gr::meta::complex_like<T>) {
            const value_type mag = std::abs(input);
            const value_type lim = static_cast<value_type>(limit);
            if (mag <= lim || mag == value_type{}) {
                return input;
            }
            return input * (lim / mag);
        } else {
            const T lim = static_cast<T>(limit);
            return std::clamp(input, -lim, lim);
        }
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_LIMITER_HPP
