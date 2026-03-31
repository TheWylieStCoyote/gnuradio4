#ifndef GNURADIO_CORRELATION_HPP
#define GNURADIO_CORRELATION_HPP

#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Correlation, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Correlation : gr::Block<Correlation<T>> {
    using Description = Doc<R""(
@brief Sliding cross-correlation between two input streams.

Per sample: `out[n] = (1/W) · Σ_{k=0}^{W-1} signal[n-k] · conj(reference[n-k])` where W = `window_size`.
Output is the normalised correlation energy at lag zero — a measure of similarity between
the two streams over the recent window. Commonly used for preamble detection, time-of-arrival
estimation, and matched filtering.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  signal;
    PortIn<T>  reference;
    PortOut<T> out;

    Annotated<gr::Size_t, "window_size", Doc<"number of samples in the sliding correlation window (≥ 1)">, Visible> window_size{32U};

    GR_MAKE_REFLECTABLE(Correlation, signal, reference, out, window_size);

    HistoryBuffer<T> _sigHistory{32};
    HistoryBuffer<T> _refHistory{32};
    T                _runningSum{};
    std::size_t      _filledCount{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t W = static_cast<std::size_t>(window_size);
        _sigHistory  = HistoryBuffer<T>(W);
        _refHistory  = HistoryBuffer<T>(W);
        _runningSum  = T{};
        _filledCount = 0UZ;
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> sigSpan, std::span<const T> refSpan, std::span<T> outSpan) noexcept {
        const std::size_t W   = static_cast<std::size_t>(window_size);
        const value_type  inv = value_type{1} / static_cast<value_type>(W);

        for (std::size_t i = 0; i < outSpan.size(); ++i) {
            // remove oldest contribution when window is full
            if (_filledCount >= W) {
                _runningSum -= _sigHistory[W - 1] * conjT(_refHistory[W - 1]);
            }
            _sigHistory.push_front(sigSpan[i]);
            _refHistory.push_front(refSpan[i]);
            _runningSum += sigSpan[i] * conjT(refSpan[i]);
            ++_filledCount;

            outSpan[i] = _runningSum * inv;
        }
        return work::Status::OK;
    }

private:
    [[nodiscard]] static constexpr T conjT(T x) noexcept {
        if constexpr (gr::meta::complex_like<T>) return std::conj(x);
        else return x;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_CORRELATION_HPP
