#ifndef GNURADIO_WINDOW_APPLY_HPP
#define GNURADIO_WINDOW_APPLY_HPP

#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/fourier/window.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::WindowApply, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct WindowApply : gr::Block<WindowApply<T>, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Multiply each frame of `frame_size` samples by a window function.

Applies a window from `gr::algorithm::window` (Hann, Hamming, Blackman, etc.) of length
`frame_size` to consecutive non-overlapping blocks of input samples. The `window_type`
setting accepts a string matching one of the `gr::algorithm::window::Type` names.
Typical use: pre-conditioning signal frames before an FFT.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t,  "frame_size",  Doc<"samples per window frame">,              Visible> frame_size{1024U};
    Annotated<std::string, "window_type", Doc<"window type name (e.g. \"Hann\")">,      Visible> window_type{"Hann"};

    GR_MAKE_REFLECTABLE(WindowApply, in, out, frame_size, window_type);

    std::vector<T> _window{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t N = static_cast<std::size_t>(frame_size);
        this->input_chunk_size  = N;
        this->output_chunk_size = N;

        const auto& typeName = static_cast<const std::string&>(window_type);
        auto wt = gr::algorithm::window::Type::Hann; // default
        // Parse window type from string
        using WType = gr::algorithm::window::Type;
        if      (typeName == "None")            wt = WType::None;
        else if (typeName == "Rectangular")     wt = WType::Rectangular;
        else if (typeName == "Hamming")         wt = WType::Hamming;
        else if (typeName == "Hann")            wt = WType::Hann;
        else if (typeName == "HannExp")         wt = WType::HannExp;
        else if (typeName == "Blackman")        wt = WType::Blackman;
        else if (typeName == "Nuttall")         wt = WType::Nuttall;
        else if (typeName == "BlackmanHarris")  wt = WType::BlackmanHarris;
        else if (typeName == "BlackmanNuttall") wt = WType::BlackmanNuttall;
        else if (typeName == "FlatTop")         wt = WType::FlatTop;
        else if (typeName == "Kaiser")          wt = WType::Kaiser;

        _window = gr::algorithm::window::create<T>(wt, N);
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        for (std::size_t i = 0; i < inSpan.size(); ++i) {
            outSpan[i] = inSpan[i] * _window[i];
        }
        return work::Status::OK;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_WINDOW_APPLY_HPP
