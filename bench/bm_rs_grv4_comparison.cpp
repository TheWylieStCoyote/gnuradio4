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
// Output: one line per benchmark: "name  median_ns  throughput_Msa_s"
//
// Note: this file uses the gnuradio4 benchmark.hpp harness from this
// directory, which requires Boost.UT.  If Boost.UT is not available,
// replace the `benchmark::` calls with the simple timing loop below.

// Pre-include <random> for GCC ≤ 14 (uniform_random_bit_generator lives
// in <random> rather than being available via <concepts>).
#include <random>

#include <gnuradio-4.0/algorithm/rng/Xoshiro256pp.hpp>
#include <gnuradio-4.0/algorithm/signal/SignalGeneratorCore.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <numeric>
#include <print>
#include <string>
#include <vector>

// ── Simple timing harness (does not require Boost.UT) ────────────────────────

static constexpr int kRuns     = 5;
static constexpr std::size_t kN = 1'000'000;

template<typename F>
static double medianNsPerRun(F&& fn) {
    std::vector<double> times(kRuns);
    for (auto& t : times) {
        auto start = std::chrono::high_resolution_clock::now();
        fn();
        auto end = std::chrono::high_resolution_clock::now();
        t = static_cast<double>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
    }
    std::sort(times.begin(), times.end());
    return times[kRuns / 2];
}

static void report(std::string_view name, double medianNs, std::size_t n) {
    double msas = static_cast<double>(n) / medianNs * 1e3; // MSa/s
    std::println("{:<36}  {:>10.1f} ns  {:>8.2f} MSa/s", name, medianNs, msas);
}

// ── RNG benchmarks ────────────────────────────────────────────────────────────

static void benchRng() {
    // raw u64
    {
        auto med = medianNsPerRun([&] {
            gr::rng::Xoshiro256pp rng(42);
            uint64_t acc = 0;
            for (std::size_t i = 0; i < kN; ++i) {
                acc += rng();
            }
            volatile auto sink = acc;
            (void)sink;
        });
        report("rng/xoshiro_u64", med, kN);
    }

    // uniform01 double
    {
        auto med = medianNsPerRun([&] {
            gr::rng::Xoshiro256pp rng(42);
            double acc = 0.0;
            for (std::size_t i = 0; i < kN; ++i) {
                acc += rng.uniform01<double>();
            }
            volatile auto sink = acc;
            (void)sink;
        });
        report("rng/uniform01_f64", med, kN);
    }
}

// ── Signal generation benchmarks ─────────────────────────────────────────────

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
            volatile auto sink = out[0];
            (void)sink;
        });
        report(std::string("signal_gen/tone_") + std::string(label), med, kN);
    };

    runTone(SignalType::Sin,      "sin");
    runTone(SignalType::Cos,      "cos");
    runTone(SignalType::Square,   "square");
    runTone(SignalType::Saw,      "saw");
    runTone(SignalType::Triangle, "triangle");

    // Uniform noise
    {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(SignalType::UniformNoise, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 42);
            std::vector<double> out(kN);
            gen.fill(std::span<double>(out));
            volatile auto sink = out[0];
            (void)sink;
        });
        report("signal_gen/noise_uniform", med, kN);
    }

    // Gaussian noise
    {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(SignalType::GaussianNoise, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 42);
            std::vector<double> out(kN);
            gen.fill(std::span<double>(out));
            volatile auto sink = out[0];
            (void)sink;
        });
        report("signal_gen/noise_gaussian", med, kN);
    }
}

// ── FIR filter (standalone — no FilterTool.hpp dependency) ───────────────────
//
// FilterTool.hpp has non-trivial dependencies on gnuradio4 core headers.
// We implement a minimal direct-form I FIR in C++ here to provide a fair
// comparison against the Rust FirFilter.

static std::vector<double> designHannLpf(std::size_t nTaps, double cutoffNorm) {
    std::vector<double> taps(nTaps);
    const double m = static_cast<double>(nTaps - 1);
    const double fc = cutoffNorm;
    for (std::size_t i = 0; i < nTaps; ++i) {
        const double n = static_cast<double>(i) - m / 2.0;
        double sinc = (n == 0.0) ? 2.0 * fc
                                 : std::sin(2.0 * M_PI * fc * n) / (M_PI * n);
        double w = 0.5 - 0.5 * std::cos(2.0 * M_PI * static_cast<double>(i) / m);
        taps[i] = sinc * w;
    }
    return taps;
}

static void benchFir() {
    for (std::size_t nTaps : {16UZ, 64UZ, 256UZ}) {
        auto taps  = designHannLpf(nTaps, 0.1);
        auto input = std::vector<double>(kN);
        for (std::size_t i = 0; i < kN; ++i) {
            input[i] = std::sin(static_cast<double>(i) * 0.001);
        }

        auto med = medianNsPerRun([&] {
            std::vector<double> history(nTaps, 0.0);
            std::size_t pos = 0;
            double acc2 = 0.0;
            for (std::size_t s = 0; s < kN; ++s) {
                history[pos] = input[s];
                double y = 0.0;
                for (std::size_t k = 0; k < nTaps; ++k) {
                    y += taps[k] * history[(pos + nTaps - k) % nTaps];
                }
                pos = (pos + 1) % nTaps;
                acc2 += y;
            }
            volatile auto sink = acc2;
            (void)sink;
        });
        report(std::format("fir/{}_taps", nTaps), med, kN);
    }
}

// ── IIR biquad (standalone) ───────────────────────────────────────────────────

static void benchIir() {
    // 2nd-order Butterworth LPF, fc = 0.1 Nyquist
    constexpr double b0 =  0.020083365564211;
    constexpr double b1 =  0.040166731128422;
    constexpr double b2 =  0.020083365564211;
    constexpr double a1 = -1.561018075800718;
    constexpr double a2 =  0.641351538057563;

    auto input = std::vector<double>(kN);
    for (std::size_t i = 0; i < kN; ++i) {
        input[i] = std::sin(static_cast<double>(i) * 0.001);
    }

    // 1 section
    {
        auto med = medianNsPerRun([&] {
            double w1 = 0.0, w2 = 0.0, acc3 = 0.0;
            for (std::size_t s = 0; s < kN; ++s) {
                const double x = input[s];
                const double y = b0 * x + w1;
                w1 = b1 * x - a1 * y + w2;
                w2 = b2 * x - a2 * y;
                acc3 += y;
            }
            volatile auto sink = acc3;
            (void)sink;
        });
        report("iir/1_section", med, kN);
    }

    // 4 sections in cascade
    {
        auto med = medianNsPerRun([&] {
            double w1[4] = {}, w2[4] = {};
            double acc4 = 0.0;
            for (std::size_t s = 0; s < kN; ++s) {
                double x = input[s];
                for (int sec = 0; sec < 4; ++sec) {
                    const double y = b0 * x + w1[sec];
                    w1[sec] = b1 * x - a1 * y + w2[sec];
                    w2[sec] = b2 * x - a2 * y;
                    x = y;
                }
                acc4 += x;
            }
            volatile auto sink = acc4;
            (void)sink;
        });
        report("iir/4_sections", med, kN);
    }
}

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
    std::println("{:<36}  {:>12}  {:>14}", "benchmark", "median_ns", "throughput");
    std::println("{}", std::string(66, '-'));

    benchRng();
    benchSignal();
    benchFir();
    benchIir();

    return 0;
}
