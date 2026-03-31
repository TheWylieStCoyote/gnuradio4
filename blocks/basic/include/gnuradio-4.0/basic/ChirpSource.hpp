#ifndef GNURADIO_CHIRP_SOURCE_HPP
#define GNURADIO_CHIRP_SOURCE_HPP

#include <cmath>
#include <numbers>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::ChirpSource, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct ChirpSource : gr::Block<ChirpSource<T>> {
    using Description = Doc<R""(
@brief Linear frequency-modulated (chirp) signal source.

Generates a real sinusoid whose instantaneous frequency sweeps linearly from `f_start`
to `f_stop` over `sweep_length` samples, then resets and repeats. The instantaneous
phase is accumulated to avoid discontinuities:

  phase[n+1] = phase[n] + 2π · (f_start + (f_stop − f_start) · n / sweep_length) / sample_rate

All frequency and sample-rate parameters are in consistent units (e.g. Hz and samples/s).
)"">;

    PortOut<T> out;

    Annotated<T,          "f_start",      Doc<"start frequency">,                        Visible> f_start{T{0}};
    Annotated<T,          "f_stop",       Doc<"stop frequency">,                         Visible> f_stop{T{1000}};
    Annotated<T,          "sample_rate",  Doc<"sample rate in samples per second">,      Visible> sample_rate{T{48000}};
    Annotated<gr::Size_t, "sweep_length", Doc<"number of samples per frequency sweep">,  Visible> sweep_length{1024U};
    Annotated<T,          "amplitude",    Doc<"output amplitude">>                                 amplitude{T{1}};

    GR_MAKE_REFLECTABLE(ChirpSource, out, f_start, f_stop, sample_rate, sweep_length, amplitude);

    std::size_t _n{0};
    T           _phase{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _n     = 0UZ;
        _phase = T{};
    }

    [[nodiscard]] T processOne() noexcept {
        const T   fs    = static_cast<T>(f_start);
        const T   fe    = static_cast<T>(f_stop);
        const T   sr    = static_cast<T>(sample_rate);
        const T   amp   = static_cast<T>(amplitude);
        const T   L     = static_cast<T>(static_cast<std::size_t>(sweep_length));
        const T   fInst = fs + (fe - fs) * static_cast<T>(_n) / L;

        const T y       = amp * std::sin(_phase);
        _phase += static_cast<T>(2) * std::numbers::pi_v<T> * fInst / sr;
        if (_phase > std::numbers::pi_v<T>) _phase -= static_cast<T>(2) * std::numbers::pi_v<T>;

        ++_n;
        if (_n >= static_cast<std::size_t>(sweep_length)) {
            _n = 0UZ;
        }

        return y;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_CHIRP_SOURCE_HPP
