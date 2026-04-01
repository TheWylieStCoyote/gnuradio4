#ifndef GNURADIO_THROTTLE_HPP
#define GNURADIO_THROTTLE_HPP

#include <chrono>
#include <complex>
#include <thread>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::Throttle, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
struct Throttle : gr::Block<Throttle<T>> {
    using Description = Doc<R""(
@brief Wall-clock rate limiter that paces sample throughput to a target `sample_rate`.

Sleeps when the graph produces samples faster than `sample_rate` allows, preventing
output buffer overruns when connected to real-time sinks. Every `maximum_items_per_chunk`
samples the block compares elapsed wall time against the expected schedule and sleeps
until the next scheduled slot if ahead of time.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<double,     "sample_rate",            Doc<"target throughput in samples per second">, Visible> sample_rate{1e6};
    Annotated<gr::Size_t, "maximum_items_per_chunk", Doc<"samples per wall-clock check interval">>           maximum_items_per_chunk{8192U};

    GR_MAKE_REFLECTABLE(Throttle, in, out, sample_rate, maximum_items_per_chunk);

    std::chrono::steady_clock::time_point _start{};
    std::size_t                           _sampleCount{0};
    std::size_t                           _checkInterval{8192};

    void start() noexcept {
        _start       = std::chrono::steady_clock::now();
        _sampleCount = 0;
        _checkInterval = static_cast<std::size_t>(maximum_items_per_chunk);
    }

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _checkInterval = static_cast<std::size_t>(maximum_items_per_chunk);
    }

    [[nodiscard]] T processOne(T x) noexcept {
        ++_sampleCount;
        if (_checkInterval > 0 && _sampleCount % _checkInterval == 0) {
            const double expectedSec = static_cast<double>(_sampleCount) / sample_rate;
            const auto   expected    = _start + std::chrono::duration_cast<std::chrono::nanoseconds>(
                                                    std::chrono::duration<double>(expectedSec));
            if (std::chrono::steady_clock::now() < expected) {
                std::this_thread::sleep_until(expected);
            }
        }
        return x;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_THROTTLE_HPP
