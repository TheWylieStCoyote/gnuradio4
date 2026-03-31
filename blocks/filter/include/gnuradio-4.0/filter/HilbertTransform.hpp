#ifndef GNURADIO_HILBERT_TRANSFORM_HPP
#define GNURADIO_HILBERT_TRANSFORM_HPP

#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::HilbertTransform, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct HilbertTransform : gr::Block<HilbertTransform<T>> {
    using Description = Doc<R""(
@brief Produces the analytic signal from a real-valued input: `out[n] = in[n−M] + j·H{in}[n]`.

Implements the Hilbert transform as an odd-symmetric Type-III FIR filter of length `n_taps`
(must be odd) windowed by a Hamming function.  Even-indexed taps are zero; odd-indexed taps
are `2 / (π·k)` scaled by the window.

The real output is the input delayed by `(n_taps − 1) / 2` samples to align it with the FIR
output.  During the first `n_taps − 1` samples the block is in warm-up and the output is
based on the available (zero-padded) history.
)"">;

    PortIn<T>               in;
    PortOut<std::complex<T>> out;

    Annotated<gr::Size_t, "n_taps", Doc<"FIR filter length (must be odd, ≥ 3)">, Visible> n_taps{63U};

    GR_MAKE_REFLECTABLE(HilbertTransform, in, out, n_taps);

    std::vector<T>   _taps{};
    HistoryBuffer<T> _history{64};
    std::size_t      _center{31};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(n_taps) | 1U; // force odd
        _center              = (N - 1U) / 2U;
        _taps.assign(N, T{});
        _history = HistoryBuffer<T>(N);

        const T pi = std::numbers::pi_v<T>;
        for (std::size_t k = 0; k < N; ++k) {
            const auto   offset  = static_cast<int>(k) - static_cast<int>(_center);
            const T      window  = static_cast<T>(0.54) - static_cast<T>(0.46) * static_cast<T>(std::cos(T{2} * pi * static_cast<T>(k) / static_cast<T>(N - 1U)));
            if (offset != 0 && (offset & 1) != 0) {
                _taps[k] = (T{2} / (pi * static_cast<T>(offset))) * window;
            }
        }
    }

    [[nodiscard]] constexpr std::complex<T> processOne(T input) noexcept {
        _history.push_front(input);
        // imaginary part: FIR convolution with Hilbert taps
        T imag{};
        for (std::size_t k = 0; k < _taps.size(); ++k) {
            imag += _taps[k] * _history[k];
        }
        // real part: input delayed by _center samples
        const T real = _history[_center];
        return {real, imag};
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_HILBERT_TRANSFORM_HPP
