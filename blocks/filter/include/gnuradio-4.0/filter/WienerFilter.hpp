#ifndef GNURADIO_WIENER_FILTER_HPP
#define GNURADIO_WIENER_FILTER_HPP

#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::WienerFilter, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct WienerFilter : gr::Block<WienerFilter<T>> {
    using Description = Doc<R""(
@brief Optimal MMSE FIR filter derived from the Wiener-Hopf normal equations.

Given a signal `in` and a reference `desired`, the block estimates the N×N autocorrelation
matrix R_xx and the N×1 cross-correlation vector r_xd from `training_length` sample pairs,
then solves R_xx·h = r_xd once via Gaussian elimination with partial pivoting.  After
training the resulting FIR taps are frozen and `in` is filtered using them; `desired` is
still consumed but ignored in this phase.  Output is zero during the training phase.

Typical uses: supervised denoising, channel equalisation, noise cancellation (where `desired`
carries a reference noise correlated with the interference in `in`), and system identification.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortIn<T>  desired;
    PortOut<T> out;

    Annotated<gr::Size_t, "n_taps",          Doc<"FIR filter length N">,                                         Visible> n_taps{16U};
    Annotated<gr::Size_t, "training_length", Doc<"number of (in, desired) sample pairs used to estimate taps">,  Visible> training_length{256U};
    Annotated<float,      "regularisation",  Doc<"diagonal loading added to R_xx before solve (≥ 0)">,           Visible> regularisation{1e-6f};

    GR_MAKE_REFLECTABLE(WienerFilter, in, desired, out, n_taps, training_length, regularisation);

    HistoryBuffer<T> _xHistory{16};
    std::vector<T>   _Rxx{};             // N×N autocorrelation matrix, row-major, accumulated sums
    std::vector<T>   _rxd{};             // N cross-correlation values, accumulated sums
    std::vector<T>   _taps{};            // N FIR tap coefficients (set after solve)
    std::size_t      _samplesAccumulated{0};
    bool             _trained{false};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(n_taps);
        _xHistory           = HistoryBuffer<T>(N);
        _Rxx.assign(N * N, T{});
        _rxd.assign(N, T{});
        _taps.assign(N, T{});
        _samplesAccumulated = 0UZ;
        _trained            = false;
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<const T> desiredSpan, std::span<T> outSpan) noexcept {
        const std::size_t N = static_cast<std::size_t>(n_taps);
        const std::size_t M = static_cast<std::size_t>(training_length);

        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            _xHistory.push_front(inSpan[i]);

            if (!_trained) {
                for (std::size_t r = 0; r < N; ++r) {
                    for (std::size_t c = 0; c < N; ++c) {
                        _Rxx[r * N + c] += _xHistory[r] * conjT(_xHistory[c]);
                    }
                    _rxd[r] += desiredSpan[i] * conjT(_xHistory[r]);
                }
                ++_samplesAccumulated;
                if (_samplesAccumulated == M) {
                    solveTaps();
                    _trained = true;
                }
                outSpan[i] = T{};
            } else {
                T acc{};
                for (std::size_t k = 0; k < N; ++k) {
                    acc += _taps[k] * _xHistory[k];
                }
                outSpan[i] = acc;
            }
        }
        return work::Status::OK;
    }

private:
    [[nodiscard]] static constexpr T conjT(T x) noexcept {
        if constexpr (gr::meta::complex_like<T>) return std::conj(x);
        else return x;
    }

    void solveTaps() {
        const std::size_t N   = static_cast<std::size_t>(n_taps);
        const value_type  inv = value_type{1} / static_cast<value_type>(_samplesAccumulated);

        // normalise accumulated sums to estimates of expectations
        for (std::size_t i = 0; i < N * N; ++i) { _Rxx[i] = _Rxx[i] * inv; }
        for (std::size_t i = 0; i < N; ++i)     { _rxd[i] = _rxd[i] * inv; }

        // diagonal regularisation
        const T reg = static_cast<T>(static_cast<value_type>(regularisation));
        for (std::size_t i = 0; i < N; ++i) { _Rxx[i * N + i] += reg; }

        // build augmented matrix [R_xx | r_xd], size N×(N+1), row-major
        std::vector<T> aug(N * (N + 1));
        for (std::size_t i = 0; i < N; ++i) {
            for (std::size_t j = 0; j < N; ++j) {
                aug[i * (N + 1) + j] = _Rxx[i * N + j];
            }
            aug[i * (N + 1) + N] = _rxd[i];
        }

        // Gaussian elimination with partial pivoting
        for (std::size_t col = 0; col < N; ++col) {
            // find pivot row
            std::size_t pivot  = col;
            value_type  maxVal = std::abs(aug[col * (N + 1) + col]);
            for (std::size_t row = col + 1; row < N; ++row) {
                const value_type val = std::abs(aug[row * (N + 1) + col]);
                if (val > maxVal) {
                    maxVal = val;
                    pivot  = row;
                }
            }
            if (pivot != col) {
                for (std::size_t j = 0; j <= N; ++j) {
                    std::swap(aug[col * (N + 1) + j], aug[pivot * (N + 1) + j]);
                }
            }

            const T diag = aug[col * (N + 1) + col];
            if (std::abs(diag) < static_cast<value_type>(1e-30)) { continue; } // near-singular column

            for (std::size_t row = col + 1; row < N; ++row) {
                const T factor = aug[row * (N + 1) + col] / diag;
                for (std::size_t j = col; j <= N; ++j) {
                    aug[row * (N + 1) + j] -= factor * aug[col * (N + 1) + j];
                }
            }
        }

        // back substitution
        for (std::size_t i = N; i-- > 0;) {
            T sum{};
            for (std::size_t j = i + 1; j < N; ++j) {
                sum += aug[i * (N + 1) + j] * _taps[j];
            }
            const T diag = aug[i * (N + 1) + i];
            _taps[i]     = std::abs(diag) > static_cast<value_type>(1e-30) ? (aug[i * (N + 1) + N] - sum) / diag : T{};
        }
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_WIENER_FILTER_HPP
