#ifndef GNURADIO_RATIONAL_RESAMPLER_HPP
#define GNURADIO_RATIONAL_RESAMPLER_HPP

#include <algorithm>
#include <cmath>
#include <complex>
#include <numeric>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/filter/FilterTool.hpp>
#include <gnuradio-4.0/algorithm/fourier/window.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::RationalResampler, [T], [ float, double, std::complex<float>, std::complex<double> ])

/**
 * @brief Rational sample-rate converter with an anti-aliasing/imaging polyphase FIR filter bank.
 *
 * Resamples by the rational factor `interpolation / decimation` (reduced by GCD). Uses a
 * prototype lowpass FIR decomposed into `interpolation` polyphase sub-filters. If `taps` is
 * empty the prototype is auto-designed as a Kaiser-windowed sinc with ~60 dB stopband
 * attenuation and passband edge at `fractional_bw * 0.5 / max(interp, decim)`. Supports
 * real and complex types; taps are always real (applied to each complex component).
 */
template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct RationalResampler : gr::Block<RationalResampler<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Rational sample-rate converter with polyphase FIR anti-aliasing filter.

Resamples by `interpolation / decimation`. If `taps` is empty a windowed-sinc prototype
is auto-designed. The prototype is split into `interpolation` polyphase sub-filters;
exactly `decimation` input samples are consumed per `interpolation` output samples.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t,         "interpolation", Doc<"upsample factor (≥ 1)">,                Visible> interpolation{1U};
    Annotated<gr::Size_t,         "decimation",    Doc<"downsample factor (≥ 1)">,               Visible> decimation{1U};
    Annotated<std::vector<float>, "taps",          Doc<"prototype FIR taps; auto-designed if empty">>     taps{std::vector<float>{}};
    Annotated<float,              "fractional_bw", Doc<"passband edge as fraction of Nyquist of the narrower rate">> fractional_bw{0.4f};

    GR_MAKE_REFLECTABLE(RationalResampler, in, out, interpolation, decimation, taps, fractional_bw);

    std::size_t _L{1};          // reduced interpolation
    std::size_t _M{1};          // reduced decimation
    std::size_t _nPerPhase{1};  // taps per polyphase sub-filter
    std::vector<std::vector<value_type>> _phases{};  // _phases[L][nPerPhase]
    std::vector<T>           _history{};    // last nPerPhase-1 input samples (oldest first)
    std::vector<T>           _combined{};  // preallocated working buffer

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t L = static_cast<std::size_t>(interpolation);
        const std::size_t M = static_cast<std::size_t>(decimation);
        const std::size_t g = std::gcd(L, M);
        _L = (L / g < 1UZ) ? 1UZ : (L / g);
        _M = (M / g < 1UZ) ? 1UZ : (M / g);

        // build prototype taps
        std::vector<value_type> prototype;
        const auto& userTaps = taps.value;
        if (!userTaps.empty()) {
            prototype.resize(userTaps.size());
            std::ranges::transform(userTaps, prototype.begin(), [](float v) { return static_cast<value_type>(v); });
        } else {
            const double fc = static_cast<double>(fractional_bw) * 0.5 / static_cast<double>(std::max(_L, _M));
            // Kaiser formula: ~60 dB stopband, transition width = fc/4 (normalised radians → 2π factor)
            const double tw    = std::max(1e-3, fc / 4.0) * 2.0 * std::numbers::pi;
            const std::size_t nEst  = gr::filter::fir::estimateNumberOfTapsKaiser(60.0, tw);
            const std::size_t nTotal = std::max(std::size_t{_L}, ((nEst + _L - 1UZ) / _L) * _L);
            auto coeffs = gr::filter::fir::generateCoefficients<value_type>(nTotal, gr::algorithm::window::Type::Kaiser, static_cast<value_type>(fc));
            prototype = std::move(coeffs.b);
            // scale by _L so that filtering an impulse through all phases gives unity gain
            for (auto& c : prototype) {
                c *= static_cast<value_type>(_L);
            }
        }

        // pad to exact multiple of _L
        const std::size_t nRounded = ((prototype.size() + _L - 1UZ) / _L) * _L;
        prototype.resize(nRounded, value_type{});
        _nPerPhase = nRounded / _L;

        // polyphase decomposition: _phases[p][k] = prototype[p + k * _L]
        _phases.assign(_L, std::vector<value_type>(_nPerPhase, value_type{}));
        for (std::size_t p = 0; p < _L; ++p) {
            for (std::size_t k = 0; k < _nPerPhase; ++k) {
                _phases[p][k] = prototype[p + k * _L];
            }
        }

        _history.assign(_nPerPhase - 1UZ, T{});
        _combined.resize(_nPerPhase - 1UZ + _M);

        this->input_chunk_size  = static_cast<gr::Size_t>(_M);
        this->output_chunk_size = static_cast<gr::Size_t>(_L);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t N       = _nPerPhase;
        const std::size_t histLen = N - 1UZ;
        const auto        inChunk = inSpan.first(_M);   // exactly _M input samples consumed

        // build combined = [history (oldest→newest), inChunk]
        std::ranges::copy(_history, _combined.begin());
        std::ranges::copy(inChunk, _combined.begin() + static_cast<std::ptrdiff_t>(histLen));

        // compute L output samples
        for (std::size_t j = 0; j < _L; ++j) {
            const std::size_t p     = (j * _M) % _L;
            const std::size_t iHead = (j * _M) / _L;  // local input offset within inChunk
            const auto& phase = _phases[p];

            T acc{};
            for (std::size_t k = 0; k < N; ++k) {
                acc += static_cast<T>(phase[k]) * _combined[iHead + N - 1UZ - k];
            }
            outSpan[j] = acc;
        }

        // update history with the last histLen samples of inChunk (oldest first)
        if (_M >= histLen) {
            std::ranges::copy(inChunk.last(histLen), _history.begin());
        } else {
            std::copy(_history.begin() + static_cast<std::ptrdiff_t>(_M), _history.end(), _history.begin());
            std::ranges::copy(inChunk, _history.end() - static_cast<std::ptrdiff_t>(_M));
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_RATIONAL_RESAMPLER_HPP
