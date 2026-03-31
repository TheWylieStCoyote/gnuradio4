#ifndef GNURADIO_MEDIAN_FILTER_HPP
#define GNURADIO_MEDIAN_FILTER_HPP

#include <algorithm>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/HistoryBuffer.hpp>

namespace gr::blocks::filter {

GR_REGISTER_BLOCK(gr::blocks::filter::MedianFilter, [T], [ float, double ])

template<typename T>
struct MedianFilter : gr::Block<MedianFilter<T>> {
    using Description = Doc<R""(
@brief Sliding-window median filter.

For each input sample, outputs the median of the last `window_size` samples.
Outputs zero until the window is full. For even window sizes the lower of the
two middle elements is returned.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "window_size", Doc<"number of samples in the sliding window">, Visible> window_size{5U};

    GR_MAKE_REFLECTABLE(MedianFilter, in, out, window_size);

    gr::HistoryBuffer<T> _history{5UZ};
    std::vector<T>       _scratch{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t sz = static_cast<std::size_t>(window_size);
        _history             = gr::HistoryBuffer<T>(sz);
        _scratch.resize(sz);
    }

    [[nodiscard]] T processOne(T x) noexcept {
        _history.push_back(x);
        if (_history.size() < static_cast<std::size_t>(window_size)) {
            return T{};
        }
        std::ranges::copy(_history, _scratch.begin());
        const std::size_t mid = _scratch.size() / 2UZ;
        std::ranges::nth_element(_scratch, _scratch.begin() + static_cast<std::ptrdiff_t>(mid));
        return _scratch[mid];
    }
};

} // namespace gr::blocks::filter

#endif // GNURADIO_MEDIAN_FILTER_HPP
