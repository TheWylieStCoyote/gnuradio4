// bm_rs_grv4_comparison.cpp — C++ throughput baselines for rs_grv4 validation.
//
// Measures the same operations as the Rust Criterion benchmarks in
// `crates/gr-bench/benches/` so that Rust/C++ throughput ratios can be
// computed.  See `docs/validation/` for the comparison tables.
//
// Compile:
//   g++ -std=c++23 -O2 -march=native \
//       -I. -I../algorithm/include \
//       bm_rs_grv4_comparison.cpp -o bm_compare
//
// Run:
//   ./bm_compare
//
// Output: one line per benchmark: "name  median_ns  MSa/s"

#include <random>

#include <gnuradio-4.0/algorithm/rng/Xoshiro256pp.hpp>
#include <gnuradio-4.0/algorithm/signal/SignalGeneratorCore.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <numbers>
#include <numeric>
#include <string>
#include <vector>

// ── Timing harness ────────────────────────────────────────────────────────────

static constexpr int         kRuns = 5;
static constexpr std::size_t kN    = 1'000'000;

template<typename F>
static double medianNsPerRun(F&& fn) {
    std::vector<double> times(kRuns);
    for (auto& t : times) {
        auto t0 = std::chrono::high_resolution_clock::now();
        fn();
        auto t1 = std::chrono::high_resolution_clock::now();
        t = static_cast<double>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[kRuns / 2];
}

static void report(std::string_view name, double medianNs, std::size_t n) {
    double msas = static_cast<double>(n) / medianNs * 1e3;
    std::printf("%-36s  %10.1f ns  %8.2f MSa/s\n",
                std::string(name).c_str(), medianNs, msas);
}

// ── RNG ───────────────────────────────────────────────────────────────────────

static void benchRng() {
    {
        auto med = medianNsPerRun([&] {
            gr::rng::Xoshiro256pp rng(42);
            uint64_t acc = 0;
            for (std::size_t i = 0; i < kN; ++i) acc += rng();
            volatile auto sink = acc; (void)sink;
        });
        report("rng/xoshiro_u64", med, kN);
    }
    {
        auto med = medianNsPerRun([&] {
            gr::rng::Xoshiro256pp rng(42);
            float acc = 0.0f;
            for (std::size_t i = 0; i < kN; ++i) acc += rng.uniform01<float>();
            volatile auto sink = acc; (void)sink;
        });
        report("rng/uniform01_f32", med, kN);
    }
    {
        auto med = medianNsPerRun([&] {
            gr::rng::Xoshiro256pp rng(42);
            double acc = 0.0;
            for (std::size_t i = 0; i < kN; ++i) acc += rng.uniform01<double>();
            volatile auto sink = acc; (void)sink;
        });
        report("rng/uniform01_f64", med, kN);
    }
}

// ── Signal generation ─────────────────────────────────────────────────────────

static void benchSignal() {
    using namespace gr::signal;
    constexpr float kFreq = 440.0f;
    constexpr float kSr   = 44100.0f;

    auto runTone = [&](SignalType type, std::string_view label) {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(type, kFreq, kSr, 0.0f, 1.0f, 0.0f, 0);
            std::vector<double> out(kN);
            gen.fill(std::span<double>(out));
            volatile auto sink = out[0]; (void)sink;
        });
        report(std::string("signal_gen/tone_") + std::string(label), med, kN);
    };

    runTone(SignalType::Sin,      "sin");
    runTone(SignalType::Cos,      "cos");
    runTone(SignalType::FastSin,  "fast_sin");
    runTone(SignalType::FastCos,  "fast_cos");
    runTone(SignalType::Square,   "square");
    runTone(SignalType::Saw,      "saw");
    runTone(SignalType::Triangle, "triangle");

    {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(SignalType::UniformNoise, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 42);
            std::vector<double> out(kN);
            gen.fill(std::span<double>(out));
            volatile auto sink = out[0]; (void)sink;
        });
        report("signal_gen/noise_uniform", med, kN);
    }
    {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(SignalType::GaussianNoise, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 42);
            std::vector<double> out(kN);
            gen.fill(std::span<double>(out));
            volatile auto sink = out[0]; (void)sink;
        });
        report("signal_gen/noise_gaussian", med, kN);
    }
}

// ── Rotator (inline — block version couples to gr framework) ──────────────────

static void benchRotator() {
    const std::array<std::pair<const char*, float>, 3> configs{{
        {"zero_inc",     0.0f},
        {"quarter_turn", std::numbers::pi_v<float> / 2.0f},
        {"small_inc",    0.001f},
    }};

    std::vector<std::complex<float>> in(kN, {1.0f, 0.0f});
    std::vector<std::complex<float>> out(kN);

    for (auto [label, phase_inc] : configs) {
        auto med = medianNsPerRun([&] {
            std::complex<float> phasor{1.0f, 0.0f};
            const std::complex<float> delta{std::cos(phase_inc), std::sin(phase_inc)};
            for (std::size_t i = 0; i < kN; ++i) {
                out[i]  = in[i] * phasor;
                phasor *= delta;
            }
            volatile auto sink = out[0]; (void)sink;
        });
        char buf[64];
        std::snprintf(buf, sizeof(buf), "rotator/%s", label);
        report(buf, med, kN);
    }
}

// ── FIR filter ────────────────────────────────────────────────────────────────

static std::vector<double> designHannLpf(std::size_t nTaps, double cutoffNorm) {
    std::vector<double> taps(nTaps);
    const double m  = static_cast<double>(nTaps - 1);
    const double fc = cutoffNorm;
    for (std::size_t i = 0; i < nTaps; ++i) {
        const double n = static_cast<double>(i) - m / 2.0;
        double sinc    = (n == 0.0) ? 2.0 * fc : std::sin(2.0 * M_PI * fc * n) / (M_PI * n);
        double w       = 0.5 - 0.5 * std::cos(2.0 * M_PI * static_cast<double>(i) / m);
        taps[i]        = sinc * w;
    }
    return taps;
}

template<typename T>
static void runFirBench(std::size_t nTaps, const std::vector<T>& taps,
                        const std::vector<T>& input, std::string_view prefix) {
    auto med = medianNsPerRun([&] {
        std::vector<T> history(nTaps, T{0});
        std::size_t pos = 0;
        T acc{0};
        for (std::size_t s = 0; s < kN; ++s) {
            history[pos] = input[s];
            T y{0};
            for (std::size_t k = 0; k < nTaps; ++k)
                y += taps[k] * history[(pos + nTaps - k) % nTaps];
            pos = (pos + 1) % nTaps;
            acc += y;
        }
        volatile auto sink = acc; (void)sink;
    });
    char label[48];
    std::snprintf(label, sizeof(label), "%s/%zu_taps", std::string(prefix).c_str(), nTaps);
    report(label, med, kN);
}

static void benchFir() {
    for (std::size_t nTaps : {16UZ, 64UZ, 256UZ}) {
        auto tapsd = designHannLpf(nTaps, 0.1);
        std::vector<double> inputd(kN);
        for (std::size_t i = 0; i < kN; ++i)
            inputd[i] = std::sin(static_cast<double>(i) * 0.001);
        runFirBench(nTaps, tapsd, inputd, "fir");
    }
}

static void benchFirF32() {
    for (std::size_t nTaps : {16UZ, 64UZ, 256UZ}) {
        auto tapsd = designHannLpf(nTaps, 0.1);
        std::vector<float> tapsf(tapsd.begin(), tapsd.end());
        std::vector<float> inputf(kN);
        for (std::size_t i = 0; i < kN; ++i)
            inputf[i] = std::sin(static_cast<float>(i) * 0.001f);
        runFirBench(nTaps, tapsf, inputf, "fir_f32");
    }
}

// ── IIR biquad ────────────────────────────────────────────────────────────────

static void benchIir() {
    constexpr double b0 =  0.020083365564211;
    constexpr double b1 =  0.040166731128422;
    constexpr double b2 =  0.020083365564211;
    constexpr double a1 = -1.561018075800718;
    constexpr double a2 =  0.641351538057563;

    std::vector<double> input(kN);
    for (std::size_t i = 0; i < kN; ++i)
        input[i] = std::sin(static_cast<double>(i) * 0.001);

    {
        auto med = medianNsPerRun([&] {
            double w1 = 0.0, w2 = 0.0, acc = 0.0;
            for (std::size_t s = 0; s < kN; ++s) {
                const double x = input[s];
                const double y = b0 * x + w1;
                w1 = b1 * x - a1 * y + w2;
                w2 = b2 * x - a2 * y;
                acc += y;
            }
            volatile auto sink = acc; (void)sink;
        });
        report("iir/1_section", med, kN);
    }
    {
        auto med = medianNsPerRun([&] {
            double w1[4] = {}, w2[4] = {}, acc = 0.0;
            for (std::size_t s = 0; s < kN; ++s) {
                double x = input[s];
                for (int sec = 0; sec < 4; ++sec) {
                    const double y = b0 * x + w1[sec];
                    w1[sec] = b1 * x - a1 * y + w2[sec];
                    w2[sec] = b2 * x - a2 * y;
                    x = y;
                }
                acc += x;
            }
            volatile auto sink = acc; (void)sink;
        });
        report("iir/4_sections", med, kN);
    }
}

// ── Savitzky-Golay (standalone, mirrors gr_algorithm::filter::savitzky_golay) ─
//
// Compute SG smoothing coefficients via Gram polynomial / normal equations
// (small p+1 system — no SVD required).  Applied as a streaming FIR convolution
// with a circular history buffer, matching the Rust SavitzkyGolayFilter.

static std::vector<double> sgCoeffs(std::size_t window, std::size_t polyOrder) {
    const int half = static_cast<int>((window - 1) / 2);
    const int p    = static_cast<int>(polyOrder) + 1; // number of polynomial coefficients

    // Build J^T J  (p×p) and solve for first column of (J^T J)^{-1}
    // Row of design matrix for position k: [1, k, k^2, ..., k^{p-1}]
    std::vector<double> JtJ(p * p, 0.0);
    for (int k = -half; k <= half; ++k) {
        double pk = 1.0;
        std::vector<double> row(p);
        for (int j = 0; j < p; ++j) { row[j] = pk; pk *= k; }
        for (int i = 0; i < p; ++i)
            for (int j = 0; j < p; ++j)
                JtJ[i * p + j] += row[i] * row[j];
    }

    // Solve (J^T J) x = e_0  via Gaussian elimination with partial pivoting
    // e_0 = [1, 0, 0, ...] selects the constant-term coefficient → smoothing
    std::vector<double> aug(p * (p + 1), 0.0);
    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < p; ++j) aug[i * (p + 1) + j] = JtJ[i * p + j];
        aug[i * (p + 1) + p] = (i == 0) ? 1.0 : 0.0;
    }
    for (int col = 0; col < p; ++col) {
        int pivot = col;
        for (int row = col + 1; row < p; ++row)
            if (std::abs(aug[row * (p + 1) + col]) > std::abs(aug[pivot * (p + 1) + col]))
                pivot = row;
        for (int j = 0; j <= p; ++j)
            std::swap(aug[col * (p + 1) + j], aug[pivot * (p + 1) + j]);
        double piv = aug[col * (p + 1) + col];
        for (int row = col + 1; row < p; ++row) {
            double f = aug[row * (p + 1) + col] / piv;
            for (int j = col; j <= p; ++j)
                aug[row * (p + 1) + j] -= f * aug[col * (p + 1) + j];
        }
    }
    std::vector<double> c(p, 0.0);
    for (int i = p - 1; i >= 0; --i) {
        c[i] = aug[i * (p + 1) + p];
        for (int j = i + 1; j < p; ++j)
            c[i] -= aug[i * (p + 1) + j] * c[j];
        c[i] /= aug[i * (p + 1) + i];
    }

    // h_k = row_k(J) · c  for k in {-half, ..., +half}
    std::vector<double> h(window);
    for (int k = -half; k <= half; ++k) {
        double val = 0.0, pk = 1.0;
        for (int j = 0; j < p; ++j) { val += c[j] * pk; pk *= k; }
        h[k + half] = val;
    }
    return h;
}

static void benchSavitzkyGolay() {
    const std::array<std::pair<const char*, std::size_t>, 2> configs{{
        {"w11", 11UZ},
        {"w51", 51UZ},
    }};

    std::vector<double> input(kN);
    for (std::size_t i = 0; i < kN; ++i)
        input[i] = std::sin(static_cast<double>(i) * 0.001);

    for (auto [label, window] : configs) {
        const auto coeffs = sgCoeffs(window, 3);

        auto med = medianNsPerRun([&] {
            std::vector<double> history(window, 0.0);
            std::size_t pos = 0;
            double acc = 0.0;
            for (std::size_t s = 0; s < kN; ++s) {
                history[pos] = input[s];
                double y = 0.0;
                for (std::size_t k = 0; k < window; ++k)
                    y += coeffs[k] * history[(pos + window - k) % window];
                pos = (pos + 1) % window;
                acc += y;
            }
            volatile auto sink = acc; (void)sink;
        });
        char buf[32];
        std::snprintf(buf, sizeof(buf), "savitzky_golay/%s", label);
        report(buf, med, kN);
    }
}

// ── FFT (standalone radix-2 Cooley-Tukey, power-of-2 sizes) ──────────────────
//
// The gnuradio4 fft.hpp requires vir::simd / magic_enum (GCC 14+ only).
// This standalone implementation matches the algorithm complexity so the
// throughput is comparable to the Rust rustfft (also a Cooley-Tukey variant).

static void fftInplace(std::vector<std::complex<float>>& a) {
    const int n = static_cast<int>(a.size());
    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    // Cooley-Tukey butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        const float ang = -2.0f * std::numbers::pi_v<float> / static_cast<float>(len);
        const std::complex<float> wlen{std::cos(ang), std::sin(ang)};
        for (int i = 0; i < n; i += len) {
            std::complex<float> w{1.0f, 0.0f};
            for (int j = 0; j < len / 2; ++j) {
                const auto u = a[i + j];
                const auto v = a[i + j + len / 2] * w;
                a[i + j]           = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

static void benchFft() {
    for (std::size_t n : {512UZ, 1024UZ, 4096UZ, 16384UZ, 65536UZ}) {
        std::vector<std::complex<float>> buf(n);

        auto med = medianNsPerRun([&] {
            std::fill(buf.begin(), buf.end(), std::complex<float>{1.0f, 0.0f});
            fftInplace(buf);
            volatile auto sink = buf[0]; (void)sink;
        });
        char label[32];
        std::snprintf(label, sizeof(label), "fft/%zu", n);
        report(label, med, n);
    }
}

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
    std::printf("%-36s  %12s  %14s\n", "benchmark", "median_ns", "throughput");
    std::printf("%s\n", std::string(66, '-').c_str());

    benchRng();
    benchSignal();
    benchRotator();
    benchFir();
    benchFirF32();
    benchIir();
    benchSavitzkyGolay();
    benchFft();

    return 0;
}
