#ifndef GNURADIO_KALMAN_FILTER_HPP
#define GNURADIO_KALMAN_FILTER_HPP

#include <cmath>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::KalmanFilter, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct KalmanFilter : gr::Block<KalmanFilter<T>> {
    using Description = Doc<R""(
@brief Scalar time-varying Kalman filter.

Models a scalar process  x[n+1] = A·x[n] + w[n]  observed as  z[n] = C·x[n] + v[n],
where w ~ N(0, Q) and v ~ N(0, R). Unlike `SteadyStateKalman`, the covariance P[n]
is propagated explicitly each sample:

  Predict:  P⁻[n] = A²·P[n-1] + Q
  Update:   K[n]  = P⁻[n]·C / (C²·P⁻[n] + R)
            x̂[n]  = A·x̂[n-1] + K[n]·(z[n] − C·A·x̂[n-1])
            P[n]  = (1 − K[n]·C) · P⁻[n]

Useful when the process is non-stationary or when transient behaviour during filter
warm-up matters. For time-invariant systems that have reached steady state, prefer
`SteadyStateKalman`.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "A",    Doc<"state transition coefficient">,       Visible> A{T{1}};
    Annotated<T, "C",    Doc<"observation coefficient">,            Visible> C{T{1}};
    Annotated<T, "Q",    Doc<"process noise variance (≥ 0)">,      Visible> Q{static_cast<T>(0.1)};
    Annotated<T, "R",    Doc<"observation noise variance (> 0)">           > R{T{1}};
    Annotated<T, "P0",   Doc<"initial state covariance estimate">>          P0{T{1}};

    GR_MAKE_REFLECTABLE(KalmanFilter, in, out, A, C, Q, R, P0);

    T _P{};     // running covariance estimate
    T _xHat{};  // running state estimate

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _P    = static_cast<T>(P0);
        _xHat = T{};
    }

    [[nodiscard]] T processOne(T z) noexcept {
        const T a    = static_cast<T>(A);
        const T c    = static_cast<T>(C);
        const T q    = static_cast<T>(Q);
        const T r    = static_cast<T>(R);

        // Predict
        const T xPre = a * _xHat;
        const T pPre = a * a * _P + q;

        // Update
        const T den  = c * pPre * c + r;
        const T K    = (std::abs(den) > static_cast<T>(1e-30)) ? (pPre * c / den) : T{};
        _xHat        = xPre + K * (z - c * xPre);
        _P           = (T{1} - K * c) * pPre;

        return _xHat;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_KALMAN_FILTER_HPP
