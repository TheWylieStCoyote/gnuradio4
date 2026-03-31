#ifndef GNURADIO_GRID_FREQUENCY_ESTIMATOR_HPP
#define GNURADIO_GRID_FREQUENCY_ESTIMATOR_HPP

#include <cmath>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::electrical {

GR_REGISTER_BLOCK(gr::electrical::GridFrequencyEstimator, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct GridFrequencyEstimator : gr::Block<GridFrequencyEstimator<T>> {
    using Description = Doc<R""(
@brief Estimate the grid fundamental frequency from zero-crossing intervals.

Detects positive-going zero crossings (negative-to-positive transitions) and
estimates the instantaneous frequency as the reciprocal of the crossing period.
The frequency estimate is updated once per cycle. Between crossings the previous
estimate is held. Outputs one frequency estimate per input sample.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<T, "sample_rate",    Doc<"signal sample rate">, Unit<"Hz">>            sample_rate{10000};
    Annotated<T, "nominal_freq",   Doc<"nominal grid frequency (50 or 60 Hz)">>      nominal_freq{50};
    Annotated<T, "max_deviation",  Doc<"maximum acceptable deviation from nominal">> max_deviation{5};

    GR_MAKE_REFLECTABLE(GridFrequencyEstimator, in, out, sample_rate, nominal_freq, max_deviation);

    T           _prevSample{};
    std::size_t _samplesSinceCrossing{0};
    T           _freqEstimate{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _prevSample          = T{};
        _samplesSinceCrossing = 0UZ;
        _freqEstimate        = static_cast<T>(nominal_freq);
    }

    [[nodiscard]] T processOne(T x) noexcept {
        ++_samplesSinceCrossing;

        // positive-going zero crossing
        if (_prevSample <= T{} && x > T{} && _samplesSinceCrossing > 1UZ) {
            const T period = static_cast<T>(_samplesSinceCrossing) / static_cast<T>(sample_rate);
            if (period > T{}) {
                const T candidate = T{1} / period;
                const T nom       = static_cast<T>(nominal_freq);
                const T dev       = static_cast<T>(max_deviation);
                if (std::abs(candidate - nom) <= dev) {
                    _freqEstimate = candidate;
                }
            }
            _samplesSinceCrossing = 0UZ;
        }

        _prevSample = x;
        return _freqEstimate;
    }
};

} // namespace gr::electrical

#endif // GNURADIO_GRID_FREQUENCY_ESTIMATOR_HPP
