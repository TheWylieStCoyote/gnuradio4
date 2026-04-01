#ifndef GNURADIO_CLOCK_RECOVERY_MM_HPP
#define GNURADIO_CLOCK_RECOVERY_MM_HPP

#include <cmath>
#include <complex>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::demod {

GR_REGISTER_BLOCK(gr::blocks::demod::ClockRecoveryMM, [T], [ float, double ])

template<typename T>
requires std::floating_point<T>
struct ClockRecoveryMM : gr::Block<ClockRecoveryMM<T>, gr::Resampling<1UZ, 1UZ, false>> {
    using Description = Doc<R""(
@brief Symbol timing recovery using the Mueller-Müller algorithm.

Operates on oversampled complex baseband input. Tracks the optimum sampling
instant using the Mueller-Müller timing error detector and a 1st-order loop
filter. Outputs one complex symbol per `sps` input samples (approximately).
The actual output rate varies slightly as the timing loop converges.
)"">;

    using complex_type = std::complex<T>;

    PortIn<complex_type>  in;
    PortOut<complex_type> out;

    Annotated<T,          "sps",            Doc<"samples per symbol (nominal)">>     sps{static_cast<T>(4)};
    Annotated<T,          "gain_mu",        Doc<"timing loop gain (µ correction)">>  gain_mu{static_cast<T>(0.175)};
    Annotated<T,          "gain_omega",     Doc<"frequency loop gain (ω correction)">> gain_omega{static_cast<T>(0.25) * static_cast<T>(0.175) * static_cast<T>(0.175)};
    Annotated<T,          "omega_rel_limit",Doc<"relative limit on ω deviation">>    omega_rel_limit{static_cast<T>(0.005)};

    GR_MAKE_REFLECTABLE(ClockRecoveryMM, in, out, sps, gain_mu, gain_omega, omega_rel_limit);

    T            _omega{};       // current samples-per-symbol estimate
    T            _omegaMid{};    // nominal omega
    T            _omegaLimit{};  // absolute omega limit
    T            _mu{};          // fractional timing offset [0,1)
    complex_type _lastSamp{};
    complex_type _lastOutput{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        _omegaMid   = static_cast<T>(sps);
        _omega      = _omegaMid;
        _omegaLimit = _omegaMid * static_cast<T>(omega_rel_limit);
        _mu         = static_cast<T>(0.5);
        _lastSamp   = {};
        _lastOutput = {};
        this->input_chunk_size  = 1U;
        this->output_chunk_size = 1U;
    }

    template<gr::InputSpanLike TInSpan, gr::OutputSpanLike TOutSpan>
    [[nodiscard]] gr::work::Status processBulk(TInSpan& inSpan, TOutSpan& outSpan) noexcept {
        std::size_t inPos  = 0UZ;
        std::size_t outPos = 0UZ;

        while (inPos < inSpan.size() && outPos < outSpan.size()) {
            // Advance by omega, interpolate at fractional position mu
            const std::size_t advance = static_cast<std::size_t>(_omega + _mu);
            if (inPos + advance >= inSpan.size()) break;

            const complex_type samp = inSpan[inPos + advance];

            // Mueller-Müller TED: e = sign(Re(last)) * Re(samp) - sign(Re(samp)) * Re(last)
            const T signLast = _lastOutput.real() >= T{} ? T{1} : T{-1};
            const T signSamp = samp.real() >= T{} ? T{1} : T{-1};
            const T err      = signLast * samp.real() - signSamp * _lastOutput.real();

            _omega      = std::clamp(_omega + static_cast<T>(gain_omega) * err, _omegaMid - _omegaLimit, _omegaMid + _omegaLimit);
            _mu         = std::fmod(_mu + static_cast<T>(gain_mu) * err, T{1});

            _lastOutput = samp;
            outSpan[outPos++] = samp;
            inPos += advance + 1UZ;
        }

        std::ignore = inSpan.consume(inPos);
        return gr::work::Status::OK;
    }
};

} // namespace gr::blocks::demod

#endif // GNURADIO_CLOCK_RECOVERY_MM_HPP
