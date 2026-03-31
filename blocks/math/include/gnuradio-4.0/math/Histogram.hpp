#ifndef GNURADIO_HISTOGRAM_HPP
#define GNURADIO_HISTOGRAM_HPP

#include <algorithm>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/DataSet.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::Histogram, [T], [ float, double ])

template<typename T>
struct Histogram : gr::Block<Histogram<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Amplitude histogram estimator.

Accumulates `accumulate_n` input samples into a histogram with `n_bins` uniform
bins spanning [`min_value`, `max_value`]. Emits one DataSet<T> per accumulation
window with bin centres as the axis and normalised counts as the signal.
)"">;

    PortIn<T>           in;
    PortOut<DataSet<T>> out;

    Annotated<gr::Size_t, "n_bins",       Doc<"number of histogram bins">, Visible>         n_bins{64U};
    Annotated<T,          "min_value",    Doc<"lower edge of the first bin">, Visible>       min_value{static_cast<T>(-1)};
    Annotated<T,          "max_value",    Doc<"upper edge of the last bin">, Visible>        max_value{static_cast<T>(1)};
    Annotated<gr::Size_t, "accumulate_n", Doc<"samples per output DataSet">, Visible>       accumulate_n{1024U};

    GR_MAKE_REFLECTABLE(Histogram, in, out, n_bins, min_value, max_value, accumulate_n);

    std::vector<std::size_t> _counts{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _counts.assign(static_cast<std::size_t>(n_bins), 0UZ);
        this->input_chunk_size  = accumulate_n;
        this->output_chunk_size = 1U;
    }

    [[nodiscard]] gr::work::Status processBulk(std::span<const T> inSpan, std::span<DataSet<T>> outSpan) noexcept {
        const std::size_t nBins = static_cast<std::size_t>(n_bins);
        const T           lo    = min_value;
        const T           hi    = max_value;
        const T           range = hi - lo;

        std::fill(_counts.begin(), _counts.end(), 0UZ);

        if (range > T{}) {
            for (const T x : inSpan) {
                if (x >= lo && x <= hi) {
                    const auto bin = static_cast<std::size_t>(static_cast<double>(x - lo) / static_cast<double>(range) * static_cast<double>(nBins));
                    _counts[std::min(bin, nBins - 1UZ)]++;
                }
            }
        }

        const double totalSamples = static_cast<double>(inSpan.size());

        DataSet<T>& ds  = outSpan[0];
        ds              = DataSet<T>{};
        ds.extents      = {static_cast<int32_t>(nBins)};
        ds.layout       = gr::LayoutRight{};
        ds.axis_names   = {"amplitude"};
        ds.axis_units   = {""};
        ds.axis_values.resize(1UZ);
        ds.axis_values[0].resize(nBins);
        ds.signal_names  = {"count"};
        ds.signal_values.resize(nBins);

        const T binWidth = (nBins > 0U) ? (range / static_cast<T>(nBins)) : T{};
        for (std::size_t k = 0; k < nBins; ++k) {
            ds.axis_values[0][k]  = lo + (static_cast<T>(k) + static_cast<T>(0.5)) * binWidth;
            ds.signal_values[k]   = totalSamples > 0.0 ? static_cast<T>(static_cast<double>(_counts[k]) / totalSamples) : T{};
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_HISTOGRAM_HPP
