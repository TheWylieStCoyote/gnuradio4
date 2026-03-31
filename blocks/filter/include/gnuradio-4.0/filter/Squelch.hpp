#ifndef GNURADIO_SQUELCH_HPP
#define GNURADIO_SQUELCH_HPP

#include <bit>
#include <complex>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::Squelch, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct Squelch : gr::Block<Squelch<T>> {
    using Description = Doc<R""(
@brief Power-threshold gate: passes samples when measured input power exceeds `threshold`,
otherwise outputs zeros.

Power is estimated as a sliding mean of `|x[n]|²` over the last `window_size` samples (O(1)
per sample). `threshold` is in linear power units (same as `|x|²`). To convert from dBFS,
use `threshold = 10^(threshold_dBFS / 10)`. `attack_length` and `decay_length` add
hysteresis: the gate opens only after `attack_length` consecutive above-threshold samples,
and closes only after `decay_length` consecutive below-threshold samples.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<value_type,  "threshold",     Doc<"power level for gate open/close">,             Visible> threshold{value_type{1}};
    Annotated<gr::Size_t,  "window_size",   Doc<"samples in the power estimation window">,      Visible> window_size{32U};
    Annotated<gr::Size_t,  "attack_length", Doc<"consecutive above-threshold samples to open">> attack_length{1U};
    Annotated<gr::Size_t,  "decay_length",  Doc<"consecutive below-threshold samples to close">> decay_length{1U};

    GR_MAKE_REFLECTABLE(Squelch, in, out, threshold, window_size, attack_length, decay_length);

    HistoryBuffer<value_type> _powerHistory{32};
    value_type                _runningPower{};
    std::size_t               _filledCount{0};
    bool                      _open{false};
    std::size_t               _aboveCount{0};
    std::size_t               _belowCount{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t cap = std::bit_ceil(std::max(static_cast<std::size_t>(window_size), std::size_t{1}));
        _powerHistory = HistoryBuffer<value_type>(cap);
        _runningPower = value_type{};
        _filledCount  = 0UZ;
        _open         = false;
        _aboveCount   = 0UZ;
        _belowCount   = 0UZ;
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t W      = static_cast<std::size_t>(window_size);
        const std::size_t attack = static_cast<std::size_t>(attack_length);
        const std::size_t decay  = static_cast<std::size_t>(decay_length);
        const value_type  thr    = static_cast<value_type>(threshold);

        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            const value_type power = std::real(inSpan[i] * std::conj(inSpan[i]));
            if (_filledCount >= W) {
                _runningPower -= _powerHistory[W - 1];
            }
            _runningPower += power;
            _powerHistory.push_front(power);
            ++_filledCount;

            const value_type meanPower = _runningPower / static_cast<value_type>(std::min(_filledCount, W));
            if (meanPower >= thr) {
                ++_aboveCount;
                _belowCount = 0UZ;
                if (!_open && _aboveCount >= attack) { _open = true; }
            } else {
                ++_belowCount;
                _aboveCount = 0UZ;
                if (_open && _belowCount >= decay) { _open = false; }
            }

            outSpan[i] = _open ? inSpan[i] : T{};
        }
        return work::Status::OK;
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_SQUELCH_HPP
