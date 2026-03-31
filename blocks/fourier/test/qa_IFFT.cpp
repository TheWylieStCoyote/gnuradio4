#include <boost/ut.hpp>
#include <cmath>
#include <complex>
#include <numbers>
#include <vector>

#include <gnuradio-4.0/algorithm/fourier/fft.hpp>
#include <gnuradio-4.0/fourier/ifft.hpp>

int main() { /* tests auto-register via boost::ut */ }

const boost::ut::suite<"IFFT"> tests = [] {
    using namespace boost::ut;
    using namespace gr::blocks::fft;

    "IFFT(FFT(x)) roundtrip"_test = []<typename T> {
        using complex_type = std::complex<T>;
        constexpr std::size_t N = 8;

        // Build a simple time-domain signal
        std::vector<complex_type> x(N);
        for (std::size_t n = 0; n < N; ++n) {
            x[n] = complex_type{static_cast<T>(n), T{}};
        }

        // Forward FFT
        gr::algorithm::FFT<complex_type, complex_type> fwdFft{};
        std::vector<complex_type>                      X(N);
        fwdFft.compute(x, X);

        // IFFT block
        IFFT<T> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size = static_cast<gr::Size_t>(N);
        block.settingsChanged({}, {});

        std::vector<complex_type> y(N);
        std::ignore = block.processBulk(std::span<const complex_type>{X}, std::span<complex_type>{y});

        for (std::size_t n = 0; n < N; ++n) {
            expect(approx(static_cast<double>(y[n].real()), static_cast<double>(x[n].real()), 1e-5)) << "real roundtrip at n=" << n;
            expect(approx(static_cast<double>(y[n].imag()), 0.0, 1e-5)) << "imag roundtrip at n=" << n;
        }
    } | std::tuple<float, double>{};

    "DC bin only → constant output"_test = [] {
        using complex_type = std::complex<float>;
        constexpr std::size_t N = 4;

        // X = [N, 0, 0, 0] → IFFT should give x = [1, 1, 1, 1]
        std::vector<complex_type> X(N, complex_type{});
        X[0] = complex_type{static_cast<float>(N), 0.f};

        IFFT<float> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size = static_cast<gr::Size_t>(N);
        block.settingsChanged({}, {});

        std::vector<complex_type> y(N);
        std::ignore = block.processBulk(std::span<const complex_type>{X}, std::span<complex_type>{y});

        for (std::size_t n = 0; n < N; ++n) {
            expect(approx(static_cast<double>(y[n].real()), 1.0, 1e-5)) << "DC output at n=" << n;
            expect(approx(static_cast<double>(y[n].imag()), 0.0, 1e-5));
        }
    };

    "settingsChanged updates chunk sizes"_test = [] {
        IFFT<double> block{};
        block.settings().init();
        std::ignore = block.settings().applyStagedParameters();
        block.fft_size = 256U;
        block.settingsChanged({}, {});

        expect(eq(static_cast<std::size_t>(block.input_chunk_size), std::size_t{256}));
        expect(eq(static_cast<std::size_t>(block.output_chunk_size), std::size_t{256}));
    };
};
