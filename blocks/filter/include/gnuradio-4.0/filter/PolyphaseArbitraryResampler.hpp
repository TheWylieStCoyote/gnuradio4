#ifndef GNURADIO_POLYPHASE_ARBITRARY_RESAMPLER_HPP
#define GNURADIO_POLYPHASE_ARBITRARY_RESAMPLER_HPP

#include <algorithm>
#include <cmath>
#include <complex>
#include <ranges>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/filter/FilterTool.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::PolyphaseArbitraryResampler, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct PolyphaseArbitraryResampler : gr::Block<PolyphaseArbitraryResampler<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Arbitrary-rate resampler using a polyphase filter bank with linear interpolation.

Converts between sample rates at an arbitrary (non-rational) ratio. A prototype lowpass FIR
is decomposed into `n_filters` polyphase phases; for each output sample, the correct phase is
selected by the fractional input position and linear interpolation is applied between adjacent
phases. Auto-designs the prototype lowpass FIR when `taps` is empty.

The block processes `n_filters` input samples and produces `round(n_filters * rate)` output
samples per call. Supports dynamically updated `rate` between calls via `settingsChanged`.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<double,             "rate",         Doc<"output/input sample-rate ratio (> 0)">, Visible> rate{1.0};
    Annotated<gr::Size_t,         "n_filters",    Doc<"number of polyphase phases (higher = lower interpolation error)">> n_filters{32U};
    Annotated<std::vector<float>, "taps",         Doc<"prototype FIR taps (auto-designed if empty)">>                    taps{};
    Annotated<float,              "fractional_bw", Doc<"normalised one-sided bandwidth of the prototype filter (0–1)">>  fractional_bw{0.4f};

    GR_MAKE_REFLECTABLE(PolyphaseArbitraryResampler, in, out, rate, n_filters, taps, fractional_bw);

    std::size_t _nFilters{32};
    std::size_t _nPerFilter{1};
    std::size_t _nIn{32};
    std::size_t _nOut{32};
    std::vector<std::vector<value_type>> _phases{};
    std::vector<T>                       _history{};  // _nPerFilter - 1 most-recent samples, oldest first
    std::vector<T>                       _combined{}; // _history ++ input chunk

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _nFilters = static_cast<std::size_t>(n_filters);

        // build prototype FIR
        const auto& userTaps = static_cast<const std::vector<float>&>(taps);
        std::vector<float> proto;
        if (userTaps.empty()) {
            const float  cutoff = static_cast<float>(fractional_bw) * std::min(1.0f, static_cast<float>(rate));
            const double tw     = 0.1 / static_cast<double>(_nFilters);
            const std::size_t nTaps = gr::filter::fir::estimateNumberOfTapsKaiser(60.0, tw);
            const auto coeff = gr::filter::fir::generateCoefficients<float>(nTaps, gr::algorithm::window::Type::Kaiser, cutoff);
            proto = coeff.b;
        } else {
            proto = userTaps;
        }

        // zero-pad to a multiple of _nFilters
        while (proto.size() % _nFilters != 0) proto.push_back(0.0f);
        _nPerFilter = proto.size() / _nFilters;

        // polyphase decomposition: _phases[p][k] = proto[p + k * _nFilters]
        _phases.assign(_nFilters + 1, std::vector<value_type>(_nPerFilter, value_type{}));
        for (std::size_t p = 0; p < _nFilters; ++p) {
            for (std::size_t k = 0; k < _nPerFilter; ++k) {
                _phases[p][k] = static_cast<value_type>(proto[p + k * _nFilters]);
            }
        }
        // phase[_nFilters] = phase[0] (wrap-around for interpolation at last phase)
        _phases[_nFilters] = _phases[0];

        _nIn  = _nFilters;
        _nOut = std::max(std::size_t{1}, static_cast<std::size_t>(std::round(static_cast<double>(_nIn) * rate)));

        _history.assign(_nPerFilter - 1, T{});
        _combined.resize(_nPerFilter - 1 + _nIn);

        this->input_chunk_size  = static_cast<gr::Size_t>(_nIn);
        this->output_chunk_size = static_cast<gr::Size_t>(_nOut);
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t histLen = _nPerFilter - 1;

        // build combined history + input
        std::ranges::copy(_history, _combined.begin());
        std::ranges::copy(inSpan.first(_nIn), _combined.begin() + static_cast<std::ptrdiff_t>(histLen));

        const double rateD    = static_cast<double>(rate);
        const double nFiltersD = static_cast<double>(_nFilters);

        for (std::size_t j = 0; j < _nOut; ++j) {
            // fractional input position for output j
            const double t      = static_cast<double>(j) / rateD;
            const std::size_t   idx    = static_cast<std::size_t>(t);
            const double frac   = t - static_cast<double>(idx);

            // polyphase selection and linear interpolation weight
            const double        phaseD = frac * nFiltersD;
            const std::size_t   p      = static_cast<std::size_t>(phaseD);
            const value_type    w      = static_cast<value_type>(phaseD - static_cast<double>(p));

            // FIR convolution for phase p (and p+1 for interpolation)
            // input history is oldest-first in _combined; newest is _combined[histLen + idx]
            value_type y0{};
            value_type y1{};
            for (std::size_t k = 0; k < _nPerFilter; ++k) {
                const value_type s = static_cast<value_type>(std::real(_combined[idx + histLen - k]));
                y0 += _phases[p][k]     * s;
                y1 += _phases[p + 1][k] * s;
            }
            // for complex input, include imaginary part
            if constexpr (gr::meta::complex_like<T>) {
                value_type y0i{};
                value_type y1i{};
                for (std::size_t k = 0; k < _nPerFilter; ++k) {
                    const value_type s = static_cast<value_type>(std::imag(_combined[idx + histLen - k]));
                    y0i += _phases[p][k]     * s;
                    y1i += _phases[p + 1][k] * s;
                }
                outSpan[j] = T{y0 + w * (y1 - y0), y0i + w * (y1i - y0i)};
            } else {
                outSpan[j] = static_cast<T>(y0 + w * (y1 - y0));
            }
        }

        // update history: take last histLen samples from _combined
        if (histLen > 0) {
            std::ranges::copy(_combined | std::views::drop(static_cast<std::ptrdiff_t>(_nIn)),
                              _history.begin());
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_POLYPHASE_ARBITRARY_RESAMPLER_HPP
