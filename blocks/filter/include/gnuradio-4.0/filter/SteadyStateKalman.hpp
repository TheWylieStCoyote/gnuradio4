#ifndef GNURADIO_STEADY_STATE_KALMAN_HPP
#define GNURADIO_STEADY_STATE_KALMAN_HPP

#include <cmath>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::SteadyStateKalman, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct SteadyStateKalman : gr::Block<SteadyStateKalman<T>> {
    using Description = Doc<R""(
@brief Scalar steady-state Kalman filter (fixed Kalman gain).

Models a scalar process  x[n+1] = A·x[n] + w[n]  observed as  z[n] = C·x[n] + v[n],
where process noise w ~ N(0, Q) and observation noise v ~ N(0, R). The steady-state
covariance P∞ is found by solving the discrete algebraic Riccati equation (DARE) once
in `settingsChanged`. The resulting fixed Kalman gain K∞ = A·P∞·Cᵀ / (C·P∞·Cᵀ + R) is
then used for every sample: x̂[n|n] = A·x̂[n−1|n−1] + K∞·(z[n] − C·A·x̂[n−1|n−1]).
Suitable for stationary linear systems where the model parameters do not change.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "A", Doc<"state transition coefficient">,      Visible> A{T{1}};
    Annotated<T, "C", Doc<"observation coefficient">,           Visible> C{T{1}};
    Annotated<T, "Q", Doc<"process noise variance (≥ 0)">,     Visible> Q{static_cast<T>(0.1)};
    Annotated<T, "R", Doc<"observation noise variance (> 0)">          > R{T{1}};

    GR_MAKE_REFLECTABLE(SteadyStateKalman, in, out, A, C, Q, R);

    T _gain{};      // steady-state Kalman gain K∞
    T _xHat{};      // current state estimate x̂[n|n]

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const T a = static_cast<T>(A);
        const T c = static_cast<T>(C);
        const T q = static_cast<T>(Q);
        const T r = static_cast<T>(R);

        // Solve scalar DARE: P = A²P - A²P·C²·P / (C²·P + R) + Q
        // Rearranged to quadratic in P:
        //   (A²-1 - C²/R · A²) · P² + (Q + R - A²·R/R + R/R·A²)·P - Q·R = 0  (after algebra)
        //
        // Standard scalar DARE: P = A²·P·(1 - C²·P/(C²·P+R)) + Q
        // Iterate until convergence (Newton method is not needed for scalar case).
        T P = q; // initial guess
        for (int iter = 0; iter < 1000; ++iter) {
            const T c2P = c * c * P;
            const T Pnew = a * a * P * (r / (c2P + r)) + q;
            if (std::abs(Pnew - P) < static_cast<T>(1e-12) * (T{1} + std::abs(P))) {
                P = Pnew;
                break;
            }
            P = Pnew;
        }

        // K∞ = A · P · Cᵀ / (C · P · Cᵀ + R)   (scalar: Cᵀ = C)
        const T den = c * P * c + r;
        _gain = (std::abs(den) > static_cast<T>(1e-30)) ? (a * P * c / den) : T{};
        _xHat = T{};
    }

    [[nodiscard]] T processOne(T z) noexcept {
        const T a    = static_cast<T>(A);
        const T c    = static_cast<T>(C);
        const T xPre = a * _xHat;                // predict: x̂[n|n-1]
        _xHat        = xPre + _gain * (z - c * xPre); // update
        return _xHat;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_STEADY_STATE_KALMAN_HPP
