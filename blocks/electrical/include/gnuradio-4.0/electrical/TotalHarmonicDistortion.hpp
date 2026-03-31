#ifndef GNURADIO_TOTAL_HARMONIC_DISTORTION_HPP
#define GNURADIO_TOTAL_HARMONIC_DISTORTION_HPP

#include <cmath>
#include <numeric>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/DataSet.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::electrical {

GR_REGISTER_BLOCK(gr::electrical::TotalHarmonicDistortion, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct TotalHarmonicDistortion : gr::Block<TotalHarmonicDistortion<T>> {
    using Description = Doc<R""(
@brief Compute Total Harmonic Distortion (THD) from a harmonic amplitude DataSet.

Accepts a `DataSet<T>` produced by `HarmonicAnalyser`. The first signal values are
interpreted as harmonic amplitudes (index 0 = fundamental V1, index 1 = 2nd harmonic V2, …).
Outputs THD = sqrt(V2²+V3²+...+Vn²) / V1 as a scalar.
)"">;

    using DataSetT = DataSet<T>;

    PortIn<DataSetT> in;
    PortOut<T>       out;

    GR_MAKE_REFLECTABLE(TotalHarmonicDistortion, in, out);

    [[nodiscard]] T processOne(DataSetT ds) noexcept {
        const auto& amps = ds.signal_values;
        if (amps.size() < 2UZ || amps[0] == T{}) {
            return T{};
        }
        // First half of signal_values are amplitudes (HarmonicAnalyser layout)
        const std::size_t nH = amps.size() / 2UZ; // signal_values = [amp1..ampN, phase1..phaseN]
        T                 sumSq{};
        for (std::size_t h = 1; h < nH; ++h) {
            sumSq += amps[h] * amps[h];
        }
        return std::sqrt(sumSq) / amps[0];
    }
};

} // namespace gr::electrical

#endif // GNURADIO_TOTAL_HARMONIC_DISTORTION_HPP
