#ifndef GNURADIO_SYMBOL_SYNC_HPP
#define GNURADIO_SYMBOL_SYNC_HPP

#include <cmath>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::demod {

GR_REGISTER_BLOCK(gr::blocks::demod::SymbolSync, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct SymbolSync : gr::Block<SymbolSync<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Symbol timing synchroniser using the Gardner timing error detector and PI loop filter.

The Gardner TED does not require knowledge of the data symbols and works well with
BPSK/QPSK. The proportional-integral (PI) loop filter controls the sample-rate
adjustment. Nominally produces one output sample per `sps` input samples.
)"">;

    using complex_type = std::complex<T>;

    PortIn<complex_type>  in;
    PortOut<complex_type> out;

    Annotated<T, "sps",        Doc<"nominal samples per symbol">>              sps{static_cast<T>(4)};
    Annotated<T, "loop_bw",    Doc<"normalised loop bandwidth">>               loop_bw{static_cast<T>(0.045)};
    Annotated<T, "damping",    Doc<"loop damping factor">>                     damping{static_cast<T>(1.0)};

    GR_MAKE_REFLECTABLE(SymbolSync, in, out, sps, loop_bw, damping);

    T            _omega{};
    T            _omegaMid{};
    T            _mu{};
    T            _kp{};   // proportional gain
    T            _ki{};   // integral gain
    T            _integ{}; // integrator state
    complex_type _midSamp{};
    complex_type _lastSamp{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _omegaMid = static_cast<T>(sps);
        _omega    = _omegaMid;
        _mu       = T{};
        _integ    = T{};
        _midSamp  = {};
        _lastSamp = {};

        // PI gains from BnTs (loop_bw) and damping
        const T Bn    = static_cast<T>(loop_bw);
        const T zeta  = static_cast<T>(damping);
        _kp = T{4} * zeta * Bn / (_omegaMid * (T{1} + T{2} * zeta * Bn + Bn * Bn));
        _ki = T{4} * Bn * Bn / (_omegaMid * (T{1} + T{2} * zeta * Bn + Bn * Bn));

        this->input_chunk_size  = 1U;
        this->output_chunk_size = 1U;
    }

    template<gr::InputSpanLike TInSpan, gr::OutputSpanLike TOutSpan>
    [[nodiscard]] gr::work::Status processBulk(TInSpan& inSpan, TOutSpan& outSpan) noexcept {
        std::size_t inPos  = 0UZ;
        std::size_t outPos = 0UZ;

        while (inPos < inSpan.size() && outPos < outSpan.size()) {
            const std::size_t halfSps = static_cast<std::size_t>(_omega / T{2} + static_cast<T>(0.5));
            if (inPos + static_cast<std::size_t>(_omega) >= inSpan.size()) break;

            // Mid-point and end-point samples (linear interpolation from available samples)
            const complex_type curSamp = inSpan[inPos + static_cast<std::size_t>(_omega)];
            const std::size_t  midIdx  = inPos + (halfSps < static_cast<std::size_t>(_omega) ? halfSps : static_cast<std::size_t>(_omega / T{2})  );
            const complex_type midSamp = inSpan[midIdx];

            // Gardner TED: e = Re(mid) * (Re(last) - Re(cur))
            const T err = midSamp.real() * (_lastSamp.real() - curSamp.real());

            // PI loop filter
            _integ = _integ + _ki * err;
            _omega = std::clamp(_omegaMid + _kp * err + _integ,
                                _omegaMid * static_cast<T>(0.9),
                                _omegaMid * static_cast<T>(1.1));

            _lastSamp         = curSamp;
            outSpan[outPos++] = curSamp;
            inPos += static_cast<std::size_t>(_omega);
        }

        std::ignore = inSpan.consume(inPos);
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::demod

#endif // GNURADIO_SYMBOL_SYNC_HPP
