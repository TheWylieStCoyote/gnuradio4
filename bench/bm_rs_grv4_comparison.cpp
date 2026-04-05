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
    std::printf("%-44s  %10.1f ns  %8.2f MSa/s\n",
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

// ── Complex decomposition ─────────────────────────────────────────────────────

static void benchComplexOps() {
    std::vector<std::complex<float>> input(kN, {1.0f, 0.5f});
    std::vector<float> output(kN);

    {
        auto med = medianNsPerRun([&] {
            for (std::size_t i = 0; i < kN; ++i)
                output[i] = input[i].real();
            volatile auto sink = output[0]; (void)sink;
        });
        report("complex_to_real/f32", med, kN);
    }
    {
        auto med = medianNsPerRun([&] {
            for (std::size_t i = 0; i < kN; ++i)
                output[i] = std::abs(input[i]);
            volatile auto sink = output[0]; (void)sink;
        });
        report("complex_to_mag/f32", med, kN);
    }
}

// ── Expression blocks: Scale / Add ────────────────────────────────────────────

static void benchExpressionBlocks() {
    std::vector<float> in0(kN, 1.0f);
    std::vector<float> in1(kN, 1.0f);
    std::vector<float> out(kN);

    {
        auto med = medianNsPerRun([&] {
            for (std::size_t i = 0; i < kN; ++i)
                out[i] = in0[i] * 2.0f;
            volatile auto sink = out[0]; (void)sink;
        });
        report("scale/f32", med, kN);
    }
    {
        auto med = medianNsPerRun([&] {
            for (std::size_t i = 0; i < kN; ++i)
                out[i] = in0[i] + in1[i];
            volatile auto sink = out[0]; (void)sink;
        });
        report("add/f32", med, kN);
    }
}

// ── Angle conversion: RadToDeg / DegToRad ────────────────────────────────────

static void benchAngleConversion() {
    std::vector<float> input(kN);
    std::vector<float> output(kN);

    {
        constexpr float kFactor = 180.0f / std::numbers::pi_v<float>;
        std::fill(input.begin(), input.end(), 1.0f);

        auto med = medianNsPerRun([&] {
            for (std::size_t i = 0; i < kN; ++i)
                output[i] = input[i] * kFactor;
            volatile auto sink = output[0]; (void)sink;
        });
        report("rad_to_deg/f32", med, kN);
    }
    {
        constexpr float kFactor = std::numbers::pi_v<float> / 180.0f;
        std::fill(input.begin(), input.end(), 180.0f);

        auto med = medianNsPerRun([&] {
            for (std::size_t i = 0; i < kN; ++i)
                output[i] = input[i] * kFactor;
            volatile auto sink = output[0]; (void)sink;
        });
        report("deg_to_rad/f32", med, kN);
    }
}

// ── Delay (ring-buffer sample shift) ─────────────────────────────────────────

static void benchDelay() {
    std::vector<float> input(kN, 1.0f);
    std::vector<float> output(kN, 0.0f);

    for (std::size_t delay : {0UZ, 64UZ, 1024UZ}) {
        auto med = medianNsPerRun([&] {
            if (delay == 0) {
                std::copy(input.begin(), input.end(), output.begin());
            } else {
                std::vector<float> history(delay, 0.0f);
                std::size_t pos = 0;
                for (std::size_t i = 0; i < kN; ++i) {
                    output[i]    = history[pos];
                    history[pos] = input[i];
                    pos          = (pos + 1) % delay;
                }
            }
            volatile auto sink = output[0]; (void)sink;
        });
        char buf[32];
        std::snprintf(buf, sizeof(buf), "delay/%zu", delay);
        report(buf, med, kN);
    }
}

// ── Circular buffer (write + read, varying batch size) ────────────────────────

static void benchCircularBuffer() {
    for (std::size_t batch : {64UZ, 256UZ, 1024UZ, 4096UZ}) {
        const std::size_t capacity = batch * 2;
        std::vector<float> ring(capacity, 0.0f);
        std::vector<float> input(batch);
        for (std::size_t i = 0; i < batch; ++i) input[i] = static_cast<float>(i);
        std::vector<float> output(batch, 0.0f);

        auto med = medianNsPerRun([&] {
            std::size_t head = 0, tail = 0;
            // write batch
            for (std::size_t i = 0; i < batch; ++i) {
                ring[head] = input[i];
                head = (head + 1) % capacity;
            }
            // read batch
            for (std::size_t i = 0; i < batch; ++i) {
                output[i] = ring[tail];
                tail = (tail + 1) % capacity;
            }
            volatile auto sink = output[0]; (void)sink;
        });
        char buf[48];
        std::snprintf(buf, sizeof(buf), "circular_buffer/%zu", batch);
        report(buf, med, batch);
    }
}

// ── File I/O (write and read 1M f32 samples) ─────────────────────────────────

static void benchFileIo() {
    const char* kPath = "/tmp/bm_rs_grv4_compare.bin";
    std::vector<float> data(kN);
    for (std::size_t i = 0; i < kN; ++i) data[i] = static_cast<float>(i) * 0.001f;

    {
        auto med = medianNsPerRun([&] {
            FILE* f = std::fopen(kPath, "wb");
            std::fwrite(data.data(), sizeof(float), kN, f);
            std::fclose(f);
        });
        report("file_sink/f32_1M", med, kN);
    }
    {
        std::vector<float> buf(kN);
        auto med = medianNsPerRun([&] {
            FILE* f = std::fopen(kPath, "rb");
            auto nread = std::fread(buf.data(), sizeof(float), kN, f);
            std::fclose(f);
            volatile auto sink = buf[0]; (void)sink; (void)nread;
        });
        report("file_source/f32_1M", med, kN);
    }
    std::remove(kPath);
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
    char label[64];
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

// fir_f32_block: same algorithm as fir_f32 — C++ has no block-wrapper overhead.
// Reported under the block name so bench_summary.py pairs them with the Rust
// block-level benchmarks (which call process_bulk rather than the raw algorithm).
static void benchFirF32Block() {
    for (std::size_t nTaps : {16UZ, 64UZ, 256UZ}) {
        auto tapsd = designHannLpf(nTaps, 0.1);
        std::vector<float> tapsf(tapsd.begin(), tapsd.end());
        std::vector<float> inputf(kN);
        for (std::size_t i = 0; i < kN; ++i)
            inputf[i] = std::sin(static_cast<float>(i) * 0.001f);
        runFirBench(nTaps, tapsf, inputf, "fir_f32_block");
    }
}

// ── IIR biquad ────────────────────────────────────────────────────────────────

// Shared biquad coefficients: 2nd-order Butterworth LPF, fc = 0.1 * Nyquist.
static constexpr double kB0 =  0.020083365564211;
static constexpr double kB1 =  0.040166731128422;
static constexpr double kB2 =  0.020083365564211;
static constexpr double kA1 = -1.561018075800718;
static constexpr double kA2 =  0.641351538057563;

template<int NSections>
static double runIirBench(const std::vector<double>& input) {
    return medianNsPerRun([&] {
        double w1[NSections] = {}, w2[NSections] = {}, acc = 0.0;
        for (std::size_t s = 0; s < kN; ++s) {
            double x = input[s];
            for (int sec = 0; sec < NSections; ++sec) {
                const double y = kB0 * x + w1[sec];
                w1[sec] = kB1 * x - kA1 * y + w2[sec];
                w2[sec] = kB2 * x - kA2 * y;
                x = y;
            }
            acc += x;
        }
        volatile auto sink = acc; (void)sink;
    });
}

static void benchIir() {
    std::vector<double> input(kN);
    for (std::size_t i = 0; i < kN; ++i)
        input[i] = std::sin(static_cast<double>(i) * 0.001);

    report("iir/1_section",  runIirBench<1>(input), kN);
    report("iir/4_sections", runIirBench<4>(input), kN);
}

// iir_block: same algorithm as iir — C++ has no block-wrapper overhead.
// Reported under the block name to pair with Rust iir_block benchmarks.
static void benchIirBlock() {
    std::vector<double> input(kN);
    for (std::size_t i = 0; i < kN; ++i)
        input[i] = std::sin(static_cast<double>(i) * 0.001);

    report("iir_block/1_section",  runIirBench<1>(input), kN);
    report("iir_block/4_sections", runIirBench<4>(input), kN);
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

// ── SVD denoiser (standalone, mirrors gr_algorithm::filter::svd_filter) ───────
//
// Implements Hankel-matrix SVD denoising:
//   1. Build H (cols × W) where H[i,j] = signal[i+j]
//   2. Form covariance C = H^T H  (W×W)
//   3. Find top-k eigenvectors of C via power iteration + deflation
//   4. Reconstruct by projecting each Hankel row onto the k-d subspace,
//      then average along anti-diagonals
//
// Window sizes ≤ 32 keep the W×W inner loop fast; input length is 512 to match
// the Rust SVD_N constant.

static std::vector<double> powerIter(std::vector<double> A, int W, int iters = 30) {
    std::vector<double> v(W, 1.0 / std::sqrt(static_cast<double>(W)));
    for (int it = 0; it < iters; ++it) {
        std::vector<double> Av(W, 0.0);
        for (int i = 0; i < W; ++i)
            for (int j = 0; j < W; ++j)
                Av[i] += A[i * W + j] * v[j];
        double norm = 0.0;
        for (auto x : Av) norm += x * x;
        norm = std::sqrt(norm);
        if (norm < 1e-15) break;
        for (auto& x : Av) x /= norm;
        v = Av;
    }
    return v;
}

static void benchSvdFilter() {
    constexpr int kSvdN = 512;
    const std::array<std::tuple<const char*, int, int>, 2> configs{{
        {"w16_k2", 16, 2},
        {"w32_k3", 32, 3},
    }};

    std::vector<double> signal(kSvdN);
    for (int i = 0; i < kSvdN; ++i)
        signal[i] = std::sin(i * 0.05);

    for (auto [label, W, k] : configs) {
        auto med = medianNsPerRun([&] {
            const int cols = kSvdN - W + 1;

            // Build covariance matrix C = H^T H  (W×W)
            std::vector<double> C(W * W, 0.0);
            for (int row = 0; row < cols; ++row)
                for (int i = 0; i < W; ++i)
                    for (int j = 0; j < W; ++j)
                        C[i * W + j] += signal[row + i] * signal[row + j];

            // Find top-k eigenvectors via deflation
            std::vector<std::vector<double>> evecs;
            std::vector<double> residual = C;
            for (int ki = 0; ki < k; ++ki) {
                auto v = powerIter(residual, W);
                evecs.push_back(v);
                // Compute eigenvalue λ = v^T C v
                double lambda = 0.0;
                for (int i = 0; i < W; ++i)
                    for (int j = 0; j < W; ++j)
                        lambda += v[i] * C[i * W + j] * v[j];
                // Deflate: residual -= λ * v * v^T
                for (int i = 0; i < W; ++i)
                    for (int j = 0; j < W; ++j)
                        residual[i * W + j] -= lambda * v[i] * v[j];
            }

            // Reconstruct via overlap-add
            std::vector<double> out(kSvdN, 0.0);
            std::vector<double> weights(kSvdN, 0.0);
            for (int row = 0; row < cols; ++row) {
                for (const auto& v : evecs) {
                    double coeff = 0.0;
                    for (int i = 0; i < W; ++i)
                        coeff += signal[row + i] * v[i];
                    for (int i = 0; i < W; ++i) {
                        out[row + i]     += coeff * v[i];
                        weights[row + i] += 1.0;
                    }
                }
            }
            for (int i = 0; i < kSvdN; ++i)
                if (weights[i] > 0.0) out[i] /= weights[i];

            volatile auto sink = out[0]; (void)sink;
        });
        char buf[32];
        std::snprintf(buf, sizeof(buf), "svd_filter/%s", label);
        report(buf, med, kSvdN);
    }
}

// ── Full pipeline (SignalGenerator → NullSink equivalent) ─────────────────────
//
// The Rust benchmark builds a `SignalGenerator → NullSink` graph including graph
// construction and run in each Criterion iteration.  We simulate this as:
//   - init generator state  (≈ graph construction)
//   - stream N samples in chunks of 1024  (≈ scheduler dispatch)
//   - accumulate into a volatile sink  (≈ NullSink)
// chunk_size=1024 matches the Rust bench default.

static void benchFullGraph() {
    using namespace gr::signal;
    constexpr float kFreq = 440.0f;
    constexpr float kSr   = 44100.0f;

    for (std::size_t n : {1000UZ, 10000UZ, 100000UZ}) {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(SignalType::Sin, kFreq, kSr, 0.0f, 1.0f, 0.0f, 0);
            std::vector<double> buf(1024);
            double acc = 0.0;
            std::size_t remaining = n;
            while (remaining > 0) {
                std::size_t chunk = std::min(remaining, std::size_t{1024});
                buf.resize(chunk);
                gen.fill(std::span<double>(buf));
                for (auto v : buf) acc += v;
                remaining -= chunk;
            }
            volatile auto sink = acc; (void)sink;
        });
        char label[32];
        std::snprintf(label, sizeof(label), "full_graph/%zu", n);
        report(label, med, n);
    }
}

static void benchFullGraphChunkSize() {
    using namespace gr::signal;
    constexpr float      kFreq  = 440.0f;
    constexpr float      kSr    = 44100.0f;
    constexpr std::size_t kTotal = 65536;

    for (std::size_t chunk_size : {64UZ, 256UZ, 1024UZ, 4096UZ}) {
        auto med = medianNsPerRun([&] {
            SignalGeneratorCore<double> gen;
            gen.configure(SignalType::Sin, kFreq, kSr, 0.0f, 1.0f, 0.0f, 0);
            std::vector<double> buf(chunk_size);
            double acc = 0.0;
            for (std::size_t i = 0; i < kTotal; i += chunk_size) {
                gen.fill(std::span<double>(buf));
                for (auto v : buf) acc += v;
            }
            volatile auto sink = acc; (void)sink;
        });
        char label[48];
        std::snprintf(label, sizeof(label), "full_graph_chunk_size/%zu", chunk_size);
        report(label, med, kTotal);
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
    std::printf("%-44s  %12s  %14s\n", "benchmark", "median_ns", "throughput");
    std::printf("%s\n", std::string(74, '-').c_str());

    benchRng();
    benchSignal();
    benchRotator();
    benchComplexOps();
    benchExpressionBlocks();
    benchAngleConversion();
    benchDelay();
    benchCircularBuffer();
    benchFileIo();
    benchFir();
    benchFirF32();
    benchFirF32Block();
    benchIir();
    benchIirBlock();
    benchSavitzkyGolay();
    benchSvdFilter();
    benchFft();
    benchFullGraph();
    benchFullGraphChunkSize();

    return 0;
}
