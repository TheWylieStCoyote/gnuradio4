#ifndef GNURADIO_KEEP_M_IN_N_HPP
#define GNURADIO_KEEP_M_IN_N_HPP

#include <algorithm>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::KeepMInN, [T], [ float, double, std::complex<float>, std::complex<double>, std::uint8_t, std::int32_t ])

template<typename T>
struct KeepMInN : gr::Block<KeepMInN<T>, Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Forward the first M of every N samples; skip the remaining N−M.

Cycles through blocks of N input samples, forwarding the first M unchanged and
discarding the rest. Useful for downsampling with keep-M-in-N decimation or for
extracting a subset of a periodic frame. Requires M ≤ N; if M = N the block is a
pass-through. The chunk sizes are set accordingly.
)"">;

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<gr::Size_t, "m_samples", Doc<"samples to forward per group">, Visible> m_samples{1U};
    Annotated<gr::Size_t, "n_samples", Doc<"total samples per group">,      Visible> n_samples{2U};

    GR_MAKE_REFLECTABLE(KeepMInN, in, out, m_samples, n_samples);

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const std::size_t M = static_cast<std::size_t>(m_samples);
        const std::size_t N = static_cast<std::size_t>(n_samples);
        this->input_chunk_size  = N;
        this->output_chunk_size = std::min(M, N);
    }

    [[nodiscard]] work::Status processBulk(std::span<const T> inSpan, std::span<T> outSpan) noexcept {
        const std::size_t M = static_cast<std::size_t>(m_samples);
        const std::size_t N = static_cast<std::size_t>(n_samples);
        const std::size_t toCopy = std::min(M, std::min(N, inSpan.size()));
        std::copy_n(inSpan.begin(), toCopy, outSpan.begin());
        return work::Status::OK;
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_KEEP_M_IN_N_HPP
