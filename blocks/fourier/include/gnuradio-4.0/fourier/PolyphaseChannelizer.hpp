#ifndef GNURADIO_POLYPHASE_CHANNELIZER_HPP
#define GNURADIO_POLYPHASE_CHANNELIZER_HPP

#include <bit>
#include <cmath>
#include <complex>
#include <numeric>
#include <ranges>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/algorithm/filter/FilterTool.hpp>
#include <gnuradio-4.0/algorithm/fourier/fft.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::fourier {

GR_REGISTER_BLOCK(gr::blocks::fourier::PolyphaseChannelizer, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct PolyphaseChannelizer : gr::Block<PolyphaseChannelizer<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief M-channel polyphase analysis filter bank.

Splits a wideband input stream into `n_channels` equal-width sub-bands. Each output
port carries one channel at `sample_rate / n_channels`. Consumes `n_channels` input
samples and produces one output sample per port per call. A prototype lowpass FIR is
auto-designed when `taps` is empty; supply custom `taps` to override.
)"">;

    using value_type   = decltype(std::real(std::declval<T>()));
    using complex_type = std::complex<value_type>;

    PortIn<T>               in;
    std::vector<PortOut<T>> out{};

    Annotated<gr::Size_t,         "n_channels",   Doc<"number of output channels (= decimation factor)">, Visible> n_channels{4U};
    Annotated<std::vector<float>, "taps",          Doc<"prototype FIR taps (auto-designed if empty)">>              taps{};
    Annotated<float,              "fractional_bw", Doc<"normalised one-sided bandwidth per channel (0–1)">>         fractional_bw{0.4f};

    GR_MAKE_REFLECTABLE(PolyphaseChannelizer, in, out, n_channels, taps, fractional_bw);

    std::size_t _M{4};
    std::size_t _nPerPhase{1};
    std::vector<std::vector<value_type>>           _phases{};
    std::vector<T>                                 _history{};
    std::vector<complex_type>                      _branchOut{};
    std::vector<complex_type>                      _ifftOut{};
    gr::algorithm::FFT<complex_type, complex_type> _fft{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _M = static_cast<std::size_t>(n_channels);
        out.resize(_M);

        // build prototype FIR
        const auto& userTaps = static_cast<const std::vector<float>&>(taps);
        std::vector<float> proto;
        if (userTaps.empty()) {
            const double fc         = static_cast<double>(fractional_bw) / static_cast<double>(_M);
            const double tw         = 0.1 / static_cast<double>(_M);
            const std::size_t nTaps = gr::filter::fir::estimateNumberOfTapsKaiser(60.0, tw);
            const auto        coeff = gr::filter::fir::generateCoefficients<float>(nTaps, gr::algorithm::window::Type::Kaiser, static_cast<float>(fc));
            proto = coeff.b;
        } else {
            proto = userTaps;
        }

        // zero-pad to a multiple of M
        while (proto.size() % _M != 0) proto.push_back(0.0f);
        _nPerPhase = proto.size() / _M;

        // polyphase decomposition: _phases[k][n] = proto[k + n*M]
        _phases.assign(_M, std::vector<value_type>(_nPerPhase));
        for (std::size_t k = 0; k < _M; ++k) {
            for (std::size_t n = 0; n < _nPerPhase; ++n) {
                _phases[k][n] = static_cast<value_type>(proto[k + n * _M]);
            }
        }

        _history.assign(_M * _nPerPhase, T{});
        _branchOut.resize(_M);
        _ifftOut.resize(_M);

        this->input_chunk_size  = static_cast<gr::Size_t>(_M);
        this->output_chunk_size = static_cast<gr::Size_t>(1);
    }

    template<gr::InputSpanLike TInSpan, gr::OutputSpanLike TOutSpan>
    [[nodiscard]] gr::work::Status processBulk(TInSpan& inSpan, std::span<TOutSpan>& outs) noexcept {
        const std::size_t N = _nPerPhase;

        // shift history left by M, append M new samples
        std::ranges::copy(_history | std::views::drop(_M), _history.begin());
        std::ranges::copy(inSpan | std::views::take(static_cast<std::ptrdiff_t>(_M)),
                          _history.end() - static_cast<std::ptrdiff_t>(_M));

        // compute polyphase branch outputs
        for (std::size_t k = 0; k < _M; ++k) {
            T acc{};
            for (std::size_t n = 0; n < N; ++n) {
                acc += static_cast<T>(_phases[k][n]) * _history[(N - 1UZ - n) * _M + k];
            }
            _branchOut[k] = complex_type{static_cast<value_type>(std::real(acc)),
                                          static_cast<value_type>(std::imag(acc))};
        }

        // IFFT via conjugate trick: IFFT(X) = conj(FFT(conj(X))) / M
        for (auto& v : _branchOut) v = std::conj(v);
        _fft.compute(_branchOut, _ifftOut);
        const value_type invM = value_type{1} / static_cast<value_type>(_M);
        for (auto& v : _ifftOut) v = std::conj(v) * invM;

        // write one sample to each output port
        for (std::size_t m = 0; m < _M; ++m) {
            if constexpr (gr::meta::complex_like<T>) {
                outs[m][0] = T{static_cast<value_type>(_ifftOut[m].real()),
                               static_cast<value_type>(_ifftOut[m].imag())};
            } else {
                outs[m][0] = static_cast<T>(_ifftOut[m].real());
            }
        }

        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::fourier

#endif // GNURADIO_POLYPHASE_CHANNELIZER_HPP
