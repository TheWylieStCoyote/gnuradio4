#ifndef GNURADIO_BIQUAD_FILTER_HPP
#define GNURADIO_BIQUAD_FILTER_HPP

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::BiquadFilter, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct BiquadFilter : gr::Block<BiquadFilter<T>> {
    using Description = Doc<R""(
@brief Direct-form II transposed second-order IIR section (biquad).

Implements the standard biquad difference equations:
  w[n]  = x[n]   - a1·w[n-1] - a2·w[n-2]
  y[n]  = b0·w[n] + b1·w[n-1] + b2·w[n-2]

Coefficients follow the convention used by audio/DSP tools (MATLAB, SciPy signal.iirfilter).
More numerically stable than a direct-form I implementation for high-Q designs.
Multiple sections may be cascaded in a graph for higher-order filters.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "b0", Doc<"feed-forward coefficient b₀">, Visible> b0{T{1}};
    Annotated<T, "b1", Doc<"feed-forward coefficient b₁">, Visible> b1{T{0}};
    Annotated<T, "b2", Doc<"feed-forward coefficient b₂">, Visible> b2{T{0}};
    Annotated<T, "a1", Doc<"feedback coefficient a₁">,     Visible> a1{T{0}};
    Annotated<T, "a2", Doc<"feedback coefficient a₂">,     Visible> a2{T{0}};

    GR_MAKE_REFLECTABLE(BiquadFilter, in, out, b0, b1, b2, a1, a2);

    T _w1{};
    T _w2{};

    [[nodiscard]] T processOne(T x) noexcept {
        const T w  = x - static_cast<T>(a1) * _w1 - static_cast<T>(a2) * _w2;
        const T y  = static_cast<T>(b0) * w + static_cast<T>(b1) * _w1 + static_cast<T>(b2) * _w2;
        _w2        = _w1;
        _w1        = w;
        return y;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_BIQUAD_FILTER_HPP
