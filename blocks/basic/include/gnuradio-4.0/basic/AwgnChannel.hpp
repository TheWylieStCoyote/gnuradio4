#ifndef GNURADIO_AWGN_CHANNEL_HPP
#define GNURADIO_AWGN_CHANNEL_HPP

#include <random>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/BlockRegistry.hpp>
#include <gnuradio-4.0/meta/utils.hpp>

namespace gr::blocks::basic {

GR_REGISTER_BLOCK(gr::blocks::basic::AwgnChannel, [T], [ float, double, std::complex<float>, std::complex<double> ])

template<typename T>
requires(std::floating_point<T> || gr::meta::complex_like<T>)
struct AwgnChannel : gr::Block<AwgnChannel<T>> {
    using Description = Doc<R""(
@brief Additive White Gaussian Noise channel.

Adds independent Gaussian noise samples to each input sample. The noise standard
deviation `noise_stddev` is in the same units as the input. For complex inputs,
independent noise is added to real and imaginary parts, each with standard deviation
`noise_stddev / sqrt(2)` so that the total complex noise power equals `noise_stddev²`.
The PRNG is seeded from `seed`; set `seed = 0` to use a random seed per `settingsChanged`.
)"">;

    using value_type = decltype(std::real(std::declval<T>()));

    PortIn<T>  in;
    PortOut<T> out;

    Annotated<value_type,   "noise_stddev", Doc<"noise standard deviation">,             Visible> noise_stddev{static_cast<value_type>(0.1)};
    Annotated<std::uint64_t,"seed",         Doc<"PRNG seed (0 = random per reset)">>              seed{42ULL};

    GR_MAKE_REFLECTABLE(AwgnChannel, in, out, noise_stddev, seed);

    std::mt19937_64                     _rng{42U};
    std::normal_distribution<value_type> _dist{};

    void settingsChanged(const gr::property_map& /*old*/, const gr::property_map& /*newSettings*/) {
        const auto s = static_cast<std::uint64_t>(seed);
        _rng = std::mt19937_64{s != 0ULL ? s : std::random_device{}()};
        _dist = std::normal_distribution<value_type>{value_type{}, static_cast<value_type>(noise_stddev)};
    }

    [[nodiscard]] T processOne(T x) noexcept {
        if constexpr (gr::meta::complex_like<T>) {
            const value_type invSqrt2 = static_cast<value_type>(0.7071067811865476);
            const value_type sigma    = static_cast<value_type>(noise_stddev) * invSqrt2;
            std::normal_distribution<value_type> d{value_type{}, sigma};
            return x + T{d(_rng), d(_rng)};
        } else {
            return x + _dist(_rng);
        }
    }
};

} // namespace gr::blocks::basic

#endif // GNURADIO_AWGN_CHANNEL_HPP
