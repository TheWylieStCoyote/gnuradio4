#ifndef GNURADIO_ENERGY_DETECTOR_HPP
#define GNURADIO_ENERGY_DETECTOR_HPP

#include <bit>
#include <complex>
#include <string>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::EnergyDetector, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct EnergyDetector : gr::Block<EnergyDetector<T>> {
    using Description = Doc<R""(
@brief Moving-energy threshold detector: publishes a tag when signal energy crosses a threshold.

Computes a running sum of `|x[n]|²` over the last `window_size` samples (O(1) per sample).
A tag with key `tag_key` and value `true`/`false` is published on rising and falling threshold
crossings respectively. Hysteresis prevents chattering on noisy thresholds: the signal must
exceed `threshold * (1 + hysteresis)` to trigger "detected" and fall below
`threshold * (1 - hysteresis)` to trigger "lost".
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "window_size", Doc<"number of samples in the energy window (≥ 1)">, Visible>               window_size{32U};
    Annotated<value_type, "threshold",   Doc<"energy threshold in same units as |x|² (power)">, Visible>             threshold{value_type{1}};
    Annotated<value_type, "hysteresis",  Doc<"relative hysteresis fraction in [0, 1) to prevent chattering">>         hysteresis{value_type{}};
    Annotated<std::string, "tag_key",    Doc<"key for the published detection tag (empty = no tags published)">>       tag_key{"energy_detect"};

    GR_MAKE_REFLECTABLE(EnergyDetector, in, out, window_size, threshold, hysteresis, tag_key);

    HistoryBuffer<value_type> _powerHistory{32};
    value_type                _runningEnergy{};
    std::size_t               _filledCount{0};
    bool                      _detected{false};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t cap = std::bit_ceil(std::max(static_cast<std::size_t>(window_size), std::size_t{1}));
        _powerHistory         = HistoryBuffer<value_type>(cap);
        _runningEnergy        = value_type{};
        _filledCount          = 0UZ;
        _detected             = false;
    }

    [[nodiscard]] constexpr T processOne(T input) noexcept {
        const std::size_t len   = static_cast<std::size_t>(window_size);
        const value_type  power = std::real(input * std::conj(input)); // |x|²
        if (_filledCount >= len) {
            _runningEnergy -= _powerHistory[len - 1];
        }
        _runningEnergy += power;
        _powerHistory.push_front(power);
        ++_filledCount;

        if (!tag_key->empty()) {
            const value_type thr     = static_cast<value_type>(threshold);
            const value_type hyst    = static_cast<value_type>(hysteresis);
            const value_type upperTh = thr * (value_type{1} + hyst);
            const value_type lowerTh = thr * (value_type{1} - hyst);

            if (!_detected && _runningEnergy > upperTh) {
                _detected = true;
                const std::string& key = tag_key;
                gr::property_map   tagMap;
                tagMap.emplace(key, true);
                this->publishTag(std::move(tagMap), 0UZ);
            } else if (_detected && _runningEnergy < lowerTh) {
                _detected = false;
                const std::string& key = tag_key;
                gr::property_map   tagMap;
                tagMap.emplace(key, false);
                this->publishTag(std::move(tagMap), 0UZ);
            }
        }

        return input;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_ENERGY_DETECTOR_HPP
