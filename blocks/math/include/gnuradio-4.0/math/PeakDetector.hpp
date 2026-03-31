#ifndef GNURADIO_PEAK_DETECTOR_HPP
#define GNURADIO_PEAK_DETECTOR_HPP

#include <string>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::math {

GR_REGISTER_BLOCK(gr::blocks::math::PeakDetector, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct PeakDetector : gr::Block<PeakDetector<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Detects local maxima in a real-valued stream using a look-ahead window.

A sample at position n is declared a peak if it is strictly greater than all
`look_ahead` samples following it, exceeds `min_peak_height`, and is at least
`min_peak_distance` samples away from the previous detected peak. Each peak is passed
through unchanged; non-peak samples are set to zero in the output. A tag with key
`tag_key` is also published at each peak.

Introduces `look_ahead` samples of output latency.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t,  "look_ahead",        Doc<"samples to look ahead for local maximum confirmation">,   Visible> look_ahead{8U};
    Annotated<T,           "min_peak_height",   Doc<"minimum value for a peak to be reported">,               Visible> min_peak_height{T{0}};
    Annotated<gr::Size_t,  "min_peak_distance", Doc<"minimum sample spacing between consecutive peaks">,      Visible> min_peak_distance{1U};
    Annotated<std::string, "tag_key",           Doc<"tag key published at each detected peak">>                         tag_key{"peak"};

    GR_MAKE_REFLECTABLE(PeakDetector, in, out, look_ahead, min_peak_height, min_peak_distance, tag_key);

    std::vector<T> _buf{};
    std::size_t    _head{0};      // write index in circular buffer
    std::size_t    _filled{0};
    std::size_t    _samplesSincePeak{0};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t L = static_cast<std::size_t>(look_ahead) + 1UZ; // include candidate
        _buf.assign(L, T{});
        _head             = 0UZ;
        _filled           = 0UZ;
        _samplesSincePeak = static_cast<std::size_t>(min_peak_distance); // allow first peak immediately
        this->input_chunk_size  = gr::Size_t{1};
        this->output_chunk_size = gr::Size_t{1};
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t L    = static_cast<std::size_t>(look_ahead) + 1UZ;
        const std::size_t D    = static_cast<std::size_t>(min_peak_distance);
        const T           minH = static_cast<T>(min_peak_height);

        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            _buf[_head] = inSpan[i];
            _head       = (_head + 1UZ) % L;
            if (_filled < L) { ++_filled; }

            if (_filled < L) {
                outSpan[i] = T{};
                continue;
            }

            // candidate is the oldest sample in the circular buffer
            const std::size_t candidateIdx = _head; // next write slot = oldest
            const T           candidate    = _buf[candidateIdx];

            bool isPeak = (candidate >= minH) && (_samplesSincePeak >= D);
            if (isPeak) {
                for (std::size_t k = 1; k < L; ++k) {
                    if (_buf[(_head + k) % L] >= candidate) {
                        isPeak = false;
                        break;
                    }
                }
            }

            ++_samplesSincePeak;
            if (isPeak) {
                _samplesSincePeak = 0UZ;
                outSpan[i]        = candidate;
                if (!tag_key->empty()) {
                    gr::property_map tagMap;
                    tagMap.emplace(static_cast<const std::string&>(tag_key), true);
                    this->publishTag(std::move(tagMap), 0UZ);
                }
            } else {
                outSpan[i] = T{};
            }
        }
        return work::Status::OK;
    }
};

} // namespace gr::blocks::math

#endif // GNURADIO_PEAK_DETECTOR_HPP
