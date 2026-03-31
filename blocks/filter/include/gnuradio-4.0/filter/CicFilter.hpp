#ifndef GNURADIO_CIC_FILTER_HPP
#define GNURADIO_CIC_FILTER_HPP

#include <cmath>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::filter {

GR_REGISTER_BLOCK(gr::filter::CicDecimator, [T], [ float, double, std::int16_t, std::int32_t ])
GR_REGISTER_BLOCK(gr::filter::CicInterpolator, [T], [ float, double, std::int16_t, std::int32_t ])

// ─── CicDecimator ───────────────────────────────────────────────────────────

template<typename T>
struct CicDecimator : Block<CicDecimator<T>, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Cascade-Integrator-Comb (CIC) decimation filter.

Implements a classical CIC decimator: N integrator stages followed by a R:1 down-
sampler, then N comb stages each with differential delay M. The overall passband
gain is R^N; the block normalises the output by dividing by this factor. CIC
filters are multiplier-free and achieve very high rate reductions (typically 8×–256×)
at low computational cost. The aliasing stopband is −13.26 N dB per octave.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "decimation",         Doc<"decimation factor R (≥ 1)">, Visible>    decimation{8U};
    Annotated<gr::Size_t, "n_stages",           Doc<"number of integrator/comb stages (≥ 1)">, Visible> n_stages{5U};
    Annotated<gr::Size_t, "differential_delay", Doc<"comb differential delay M (≥ 1)">> differential_delay{1U};

    GR_MAKE_REFLECTABLE(CicDecimator, in, out, decimation, n_stages, differential_delay);

    std::vector<double> _integrators{};
    std::vector<double> _combs{};       // interleaved: [stage0_prev, stage0_curr, stage1_prev, ...]
    double              _gainInverse{1.0};

    void settingsChanged(const property_map& /*old*/, const property_map& /*new*/) {
        const std::size_t N = static_cast<std::size_t>(n_stages);
        const std::size_t M = static_cast<std::size_t>(differential_delay);
        _integrators.assign(N, 0.0);
        _combs.assign(N * M, 0.0);  // M delayed samples per stage
        const double R     = static_cast<double>(static_cast<std::size_t>(decimation));
        _gainInverse        = 1.0 / std::pow(R, static_cast<double>(N));
        this->input_chunk_size = decimation;
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> input, std::span<T> output) noexcept {
        const std::size_t R = static_cast<std::size_t>(decimation);
        const std::size_t N = static_cast<std::size_t>(n_stages);
        const std::size_t M = static_cast<std::size_t>(differential_delay);
        assert(output.size() >= input.size() / R);

        for (std::size_t outIdx = 0; outIdx < output.size(); ++outIdx) {
            // Integrate R samples
            for (std::size_t r = 0; r < R; ++r) {
                double v = static_cast<double>(input[outIdx * R + r]);
                for (std::size_t s = 0; s < N; ++s) {
                    _integrators[s] += v;
                    v                = _integrators[s];
                }
            }
            // Comb stages on the decimated sample
            double v = _integrators[N - 1];
            for (std::size_t s = 0; s < N; ++s) {
                // Circular shift: oldest slot holds the M-delayed sample
                const std::size_t base  = s * M;
                const double      prev  = _combs[base];
                // Shift delay line
                for (std::size_t d = 0; d < M - 1; ++d) {
                    _combs[base + d] = _combs[base + d + 1];
                }
                _combs[base + M - 1] = v;
                v -= prev;
            }
            output[outIdx] = static_cast<T>(v * _gainInverse);
        }
        return work::Status::OK;
    }
};

// ─── CicInterpolator ────────────────────────────────────────────────────────

template<typename T>
struct CicInterpolator : Block<CicInterpolator<T>, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Cascade-Integrator-Comb (CIC) interpolation filter.

Implements a classical CIC interpolator: N comb stages, then a 1:L up-sampler
(zero-insertion), then N integrator stages. The overall gain is L^N; the block
normalises by dividing by this factor. Useful for very high interpolation ratios
(typically 8×–256×) without multipliers.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "interpolation",      Doc<"interpolation factor L (≥ 1)">, Visible>  interpolation{8U};
    Annotated<gr::Size_t, "n_stages",           Doc<"number of integrator/comb stages (≥ 1)">, Visible> n_stages{5U};
    Annotated<gr::Size_t, "differential_delay", Doc<"comb differential delay M (≥ 1)">> differential_delay{1U};

    GR_MAKE_REFLECTABLE(CicInterpolator, in, out, interpolation, n_stages, differential_delay);

    std::vector<double> _combs{};
    std::vector<double> _integrators{};
    double              _gainInverse{1.0};

    void settingsChanged(const property_map& /*old*/, const property_map& /*new*/) {
        const std::size_t N = static_cast<std::size_t>(n_stages);
        const std::size_t M = static_cast<std::size_t>(differential_delay);
        _combs.assign(N * M, 0.0);
        _integrators.assign(N, 0.0);
        const double L  = static_cast<double>(static_cast<std::size_t>(interpolation));
        _gainInverse     = 1.0 / std::pow(L, static_cast<double>(N));
        this->output_chunk_size = interpolation;
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> input, std::span<T> output) noexcept {
        const std::size_t L = static_cast<std::size_t>(interpolation);
        const std::size_t N = static_cast<std::size_t>(n_stages);
        const std::size_t M = static_cast<std::size_t>(differential_delay);
        assert(output.size() >= input.size() * L);

        for (std::size_t inIdx = 0; inIdx < input.size(); ++inIdx) {
            // Comb stages
            double v = static_cast<double>(input[inIdx]);
            for (std::size_t s = 0; s < N; ++s) {
                const std::size_t base = s * M;
                const double      prev = _combs[base];
                for (std::size_t d = 0; d < M - 1; ++d) {
                    _combs[base + d] = _combs[base + d + 1];
                }
                _combs[base + M - 1] = v;
                v -= prev;
            }
            // Zero-insertion + integration for each output sample
            for (std::size_t l = 0; l < L; ++l) {
                double iv = (l == 0) ? v : 0.0;
                for (std::size_t s = 0; s < N; ++s) {
                    _integrators[s] += iv;
                    iv               = _integrators[s];
                }
                output[inIdx * L + l] = static_cast<T>(iv * _gainInverse);
            }
        }
        return work::Status::OK;
    }
};

} // namespace gr::filter

#endif // GNURADIO_CIC_FILTER_HPP
