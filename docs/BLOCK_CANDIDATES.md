# Block candidates

Proposed blocks not present in the current inventory (`BLOCK_INVENTORY.md`).
Organised by module, with rationale and suggested API for each.

Priority ratings reflect how foundational or broadly needed each block is:
- **P1** — critical gap; frequently needed, no reasonable workaround with existing blocks
- **P2** — high value; commonly used, saves significant graph complexity
- **P3** — useful addition; covers a specific but well-defined need

Blocks marked **✓ implemented** have been added to the codebase and appear in `BLOCK_INVENTORY.md`.

---

## Module: basic

### Resampling

#### `Decimator<T>` — P1 ✓ implemented
Pre-existing as `gr::filter::Decimator<T>` in `blocks/filter/include/gnuradio-4.0/filter/time_domain_filter.hpp`. Equivalent to `Keep1InN` with `offset=0`.

#### `Interpolator<T>` — P1 ✓ implemented
Implemented as `gr::filter::Interpolator<T>` in `blocks/filter/include/gnuradio-4.0/filter/time_domain_filter.hpp`. Zero-insertion upsampler using `output_chunk_size` for dynamic ratio control.

#### `RationalResampler<T>` — P1
Resample by a rational factor `interpolation / decimation` with an integrated anti-aliasing/anti-imaging FIR filter. The standard building block for sample-rate conversion between arbitrary rates (e.g. 2 Msps → 48 ksps audio). Equivalent to GNU Radio 3.x `rational_resampler`.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `interpolation`, `decimation`, `taps` (auto-designed if empty), `fractional_bw`
- **Processing:** `processBulk`

#### `PolyphaseArbitraryResampler<T>` — P2
Arbitrary (non-rational) resampling using a polyphase filter bank with linear interpolation between phases. Handles continuously varying or irrational resampling ratios. Useful for clock-recovery feedback loops.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `rate`, `n_filters`, `taps`
- **Processing:** `processBulk`

---

### Stream manipulation

#### `Keep1InN<T>` — P1 ✓ implemented
Covered by `gr::filter::Decimator<T>` (pre-existing). The `decim` setting controls N; offset support is not separate but offset=0 (first sample kept) is the standard use case.

#### `KeepMInN<T>` — P2
Forward M consecutive samples out of every N consumed. Generalises `Keep1InN` for burst-mode or sub-frame extraction.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `m`, `n`, `offset`
- **Processing:** `processBulk`

#### `StreamToVector<T>` — P1
Accumulates `vector_size` scalar samples into a single vector-typed output item. The fundamental bridge between sample-stream and block-processing domains (FFT, matrix operations, batch decoders).
- **Ports:** `PortIn<T> in`, `PortOut<std::vector<T>> out`  
- **Settings:** `vector_size`
- **Processing:** `processBulk` — `Resampling<N, 1>`

#### `VectorToStream<T>` — P1
Unpacks each input vector into `vector_size` scalar output samples. Counterpart to `StreamToVector`.
- **Ports:** `PortIn<std::vector<T>> in`, `PortOut<T> out`
- **Settings:** `vector_size`
- **Processing:** `processBulk` — `Resampling<1, N>`

#### `StreamMux<T>` — P2
Interleaves N input streams into one output stream, cycling through inputs in round-robin order with a configurable number of samples taken from each stream per cycle. Counterpart to `Selector` for ordered interleaving.
- **Ports:** `std::vector<PortIn<T>> in`, `PortOut<T> out`
- **Settings:** `n_inputs`, `samples_per_input`
- **Processing:** `processBulk`

#### `StreamDemux<T>` — P2
Splits one input stream into N output streams, cycling through outputs in round-robin order. Counterpart to `StreamMux`.
- **Ports:** `PortIn<T> in`, `std::vector<PortOut<T>> out`
- **Settings:** `n_outputs`, `samples_per_output`
- **Processing:** `processBulk`

#### `Repeat<T>` — P3 ✓ implemented
Implemented as `gr::filter::Repeat<T>` in `blocks/filter/include/gnuradio-4.0/filter/time_domain_filter.hpp`. Zero-order-hold upsampler; each input sample is repeated `repeat` times. Supports all numeric types.

---

### Tags

#### `StreamTagger<T>` — P2
Injects a configurable tag into the stream at a regular sample interval or on a one-shot basis. Useful for marking frame boundaries, injecting metadata, or triggering downstream blocks.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `tag_key`, `tag_value`, `interval` (0 = one-shot), `offset`
- **Processing:** `processBulk`

#### `TagGate<T>` — P2
Passes samples only while a named tag is active (or only when it is inactive). Provides tag-controlled gating of the data stream.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `tag_key`, `open_on_tag` (bool)
- **Processing:** `processBulk` — `NoDefaultTagForwarding`

#### `TagDebugSink<T>` — P2
Logs every tag arriving on its input to stdout or a file; passes samples through unchanged. Indispensable for debugging tag propagation in complex graphs.
- **Ports:** `PortIn<T> in`, `PortOut<T, Optional> out`
- **Settings:** `file_path` (stdout if empty), `tag_key_filter`
- **Processing:** `processBulk`

---

### Windowing

#### `WindowApply<T>` — P2
Multiplies a block of `window_size` samples by a named window function (Hann, Hamming, Blackman, Kaiser, …). Decouples windowing from the FFT block so the same window can be applied before other transforms or for signal conditioning.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size`, `window_type`, `beta` (Kaiser only)
- **Processing:** `processBulk`

---

### Flow control

#### `Throttle<T>` — P2
Limits sample throughput to a target wall-clock rate by sleeping when the graph produces samples faster than the rate allows. Essential whenever a graph feeds a real-time sink (audio device, SDR hardware, network socket) that cannot absorb samples at full CPU speed; without it the output buffer overflows or the UI becomes unresponsive.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `sample_rate`, `maximum_items_per_chunk`
- **Processing:** `processBulk`

#### `Head<T>` — P2
Passes exactly `n_samples` samples downstream then signals `requestStop()`. The fundamental primitive for finite-length simulations, unit tests that drive a graph with a fixed burst, and batch-processing pipelines.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `n_samples`
- **Processing:** `processBulk`

#### `Skip<T>` — P2
Discards the first `n_samples` samples and passes all subsequent samples unchanged. Complements `Head<T>` for windowed extraction; also useful for skipping filter warm-up before measuring steady-state output.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `n_samples`
- **Processing:** `processBulk`

---

### Sources

#### `ChirpSource<T>` — P2
Generates a linear frequency sweep from `start_frequency` to `stop_frequency` over `sweep_length` samples, then either stops or repeats. Used in radar, sonar, spread-spectrum testing, and impulse-response measurement (a chirp covers a wide bandwidth with uniform energy).
- **Ports:** `PortOut<T> out`
- **Settings:** `start_frequency`, `stop_frequency`, `sample_rate`, `sweep_length`, `repeat` (bool), `amplitude`
- **State:** `std::size_t _sample` — current position within sweep
- **Processing:** `processBulk`

#### `AwgnChannel<T>` — P2
Adds white Gaussian noise at a configurable SNR or noise standard deviation to a passthrough signal. Indispensable for simulation and BER testing; without it a noise source must be wired up and mixed manually outside the block graph.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `noise_stddev` or `snr_db` (one used, other derived), `seed`
- **State:** `std::mt19937 _rng`, `std::normal_distribution<value_type> _dist`
- **Processing:** `processOne`

---

### Framing

#### `HeaderPayloadDemux<T>` — P2
Splits a tagged stream into header and payload sections. On receiving a trigger tag (e.g. from `EnergyDetector` or `Correlation`), it forwards exactly `header_length` samples to one output port and the following `payload_length` samples to a second output port, then returns to armed state. Essential for packet radio and burst-mode decoders.
- **Ports:** `PortIn<T> in`, `PortOut<T> header`, `PortOut<T> payload`
- **Settings:** `header_length`, `payload_length`, `trigger_tag_key`
- **Processing:** `processBulk` — `NoDefaultTagForwarding`

---

## Module: math

#### `MovingAverage<T>` — P1 ✓ implemented
Implemented as `gr::blocks::math::MovingAverage<T>` in `blocks/math/include/gnuradio-4.0/math/MovingAverage.hpp`. O(1)-per-sample running sum with warm-up phase. Supports float, double, complex<float>, complex<double>.

#### `MovingRms<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::MovingRms<T>` in `blocks/math/include/gnuradio-4.0/math/MovingRms.hpp`. Output is always real `value_type`. Supports float, double, complex<float>, complex<double>.

#### `Clamp<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Clamp<T>` in `blocks/math/include/gnuradio-4.0/math/Clamp.hpp`. For complex types, real and imaginary parts are clamped independently. Supports float, double, complex<float>, complex<double>.

#### `Threshold<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Threshold<T>` in `blocks/math/include/gnuradio-4.0/math/Threshold.hpp`.

#### `SchmittTrigger<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::SchmittTrigger<T>` in `blocks/math/include/gnuradio-4.0/math/SchmittTrigger.hpp`.

#### `Goertzel<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Goertzel<T>` in `blocks/math/include/gnuradio-4.0/math/Goertzel.hpp`.

#### `PhaseUnwrap<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::PhaseUnwrap<T>` in `blocks/math/include/gnuradio-4.0/math/PhaseUnwrap.hpp`. Supports float, double.

#### `Conjugate<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Conjugate<T>` in `blocks/math/include/gnuradio-4.0/math/Conjugate.hpp`. Supports complex<float>, complex<double>.

#### `Accumulator<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Accumulator<T>` in `blocks/math/include/gnuradio-4.0/math/Accumulator.hpp`. Supports float, double, complex<float>, complex<double>.

#### `Differentiator<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Differentiator<T>` in `blocks/math/include/gnuradio-4.0/math/Differentiator.hpp`. Supports float, double, complex<float>, complex<double>.

#### `PeakDetector<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::PeakDetector<T>` in `blocks/math/include/gnuradio-4.0/math/PeakDetector.hpp`.

#### `Histogram<T>` — P3
Accumulates a sample-amplitude histogram over a configurable window and emits it as a `DataSet` output. Used for distribution analysis, ADC characterisation, and density estimation.
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<T>> out`
- **Settings:** `n_bins`, `min_value`, `max_value`, `accumulate_n`
- **Processing:** `processBulk`

#### `Correlation<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::Correlation<T>` in `blocks/math/include/gnuradio-4.0/math/Correlation.hpp`.

#### `AutoCorrelation<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::AutoCorrelation<T>` in `blocks/math/include/gnuradio-4.0/math/AutoCorrelation.hpp`.

#### `InstantaneousFrequency<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::InstantaneousFrequency<T>` in `blocks/math/include/gnuradio-4.0/math/InstantaneousFrequency.hpp`.

---

## Module: filter

#### `DCBlocker<T>` — P1 ✓ implemented
Implemented as `gr::blocks::filter::DCBlocker<T>` in `blocks/filter/include/gnuradio-4.0/filter/DCBlocker.hpp`. Uses moving-average subtraction (FIR, linear-phase) rather than single-pole IIR. Supports float, double, complex<float>, complex<double>.

#### `HilbertTransform<T>` — P1 ✓ implemented
Implemented as `gr::blocks::filter::HilbertTransform<T>` in `blocks/filter/include/gnuradio-4.0/filter/HilbertTransform.hpp`. Odd-symmetric Hamming-windowed Type-III FIR. Output is aligned `in[n-M] + j·H{in}[n]`. Supports float, double.

#### `Squelch<T>` — P2
Gates the output stream based on the measured input power: passes samples when power exceeds `threshold`, suppresses (outputs zeros or stops) otherwise. Foundational block for voice/burst radio receivers.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `threshold` (dB or linear), `attack_length`, `decay_length`
- **Processing:** `processBulk`

#### `Convolver<T>` — P2
Overlap-add or overlap-save fast convolution for large FIR kernels. Provides O(N log N) complexity where `fir_filter`'s O(N·M) is impractical for long impulse responses (room acoustics, channel equalisation).
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `taps`, `block_size`
- **Processing:** `processBulk`

#### `AdaptiveLmsFilter<T>` — P2
Least-Mean-Squares (LMS) adaptive FIR filter with a separate error/reference input. Used for echo cancellation, interference cancellation, and channel equalisation.
- **Ports:** `PortIn<T> in`, `PortIn<T> reference`, `PortOut<T> out`, `PortOut<T> error`
- **Settings:** `n_taps`, `step_size` (µ), `leak_factor`
- **Processing:** `processBulk`

#### `MedianFilter<T>` — P3
Applies a sliding-window median filter to reject impulse noise. More effective than a moving average for impulsive interference; common in measurement and instrumentation pipelines.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size`
- **Processing:** `processBulk`

#### `BiquadFilter<T>` — P2
Direct-form II transposed second-order IIR section (biquad). More numerically stable than the generic `iir_filter` for high-Q designs because it avoids accumulating round-off error in the feedback path. Accepts standard `[b0, b1, b2, a1, a2]` coefficients, enabling direct use of audio/DSP coefficient tables. Supports cascading multiple sections for higher-order filters.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `b0`, `b1`, `b2`, `a1`, `a2`
- **State:** two delay elements `_w1`, `_w2` (direct-form II transposed)
- **Processing:** `processOne` — stateless coefficient path, minimal state

#### `FractionalDelayLine<T>` — P2
Delays a signal by a non-integer number of samples using a Farrow filter (polynomial interpolation) or a Lagrange FIR. Required wherever timing alignment at sub-sample resolution is needed: clock recovery feedback, pre-matched-filter alignment, and multi-channel coherent combining.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `delay` (fractional samples), `n_taps` (filter order, default 8)
- **State:** `HistoryBuffer<T> _history`
- **Processing:** `processOne` or `processBulk`

#### `WienerFilter<T>` — P2 ✓ implemented
Implemented as `gr::blocks::filter::WienerFilter<T>` in `blocks/filter/include/gnuradio-4.0/filter/WienerFilter.hpp`. Trains on paired `(in, desired)` streams for `training_length` samples, solves R_xx·h = r_xd via Gaussian elimination with partial pivoting and diagonal regularisation, then applies frozen FIR taps. Supports float, double, complex<float>, complex<double>.

#### `KalmanFilter<T>` — P2
Recursive optimal state estimator for linear dynamic systems with Gaussian process and observation noise. Alternates between a predict step (`x̂[k|k-1] = F·x̂[k-1]`, `P[k|k-1] = F·P·F^T + Q`) and an update step (`K = P·H^T·(H·P·H^T+R)^{-1}`, `x̂ += K·(z−H·x̂)`, `P = (I−K·H)·P`). Distinct from `WienerFilter` in that it handles non-stationary dynamic models and propagates a full error-covariance matrix at every step.
- **Ports:** `PortIn<T> in` (observation z[k], length `obs_dim`), `PortOut<T> out` (state estimate x̂[k], length `state_dim`)
- **Settings:** `state_dim`, `obs_dim`, `F` (state-transition matrix, flat vector, length state_dim²), `H` (observation matrix, obs_dim×state_dim), `Q` (process noise covariance, state_dim²), `R` (observation noise covariance, obs_dim²), `initial_state` (state_dim), `initial_covariance` (state_dim²)
- **State:** `std::vector<T> _xHat` (state estimate), `std::vector<T> _P` (error-covariance matrix)
- **Processing:** `processBulk` — vector-valued input/output; each call processes one observation vector per sample

#### `SteadyStateKalman<T>` — P2
Simplified Kalman filter for time-invariant systems where the filter gain converges to a fixed steady-state value. Solves the Discrete Algebraic Riccati Equation (DARE) in `settingsChanged` to obtain the steady-state gain K∞; per-sample work is then a single matrix-vector multiply (`x̂[k] = (F − K∞·H·F)·x̂[k−1] + K∞·z[k]`) with no covariance propagation. Significantly lower per-sample cost than `KalmanFilter` when the system model is constant.
- **Ports:** `PortIn<T> in` (observation z[k], length `obs_dim`), `PortOut<T> out` (state estimate x̂[k], length `state_dim`)
- **Settings:** `state_dim`, `obs_dim`, `F`, `H`, `Q`, `R`, `initial_state` (same matrix layout as `KalmanFilter`)
- **State:** `std::vector<T> _xHat`, `std::vector<T> _K` (steady-state gain matrix, state_dim×obs_dim, computed once)
- **Processing:** `processBulk`

---

## Module: fourier

#### `IFFT<T, U, FourierAlgorithm>` — P1
Computes the Inverse (Fast) Fourier Transform. The direct counterpart to `FFT`; required for any synthesis chain (OFDM modulation, overlap-add filtering, spectral shaping).
- **Ports:** `PortIn<DataSet<T>> in`, `PortOut<U> out`
- **Settings:** `fft_size`, `window_type` (applied before transform for spectral leakage control on reconstruction)
- **Processing:** `processBulk`

#### `PolyphaseChannelizer<T>` — P2
Splits a wideband input stream into N equal-width sub-band channels using a polyphase filter bank. Each output port carries one channel at `sample_rate / N`. The standard building block for spectrum surveillance and multi-channel receivers.
- **Ports:** `PortIn<T> in`, `std::vector<PortOut<T>> out`
- **Settings:** `n_channels`, `taps`, `oversample_rate`
- **Processing:** `processBulk`

#### `SpectralEstimator<T>` — P2
Estimates the power spectral density using Welch's method: overlapping windowed FFT frames are averaged to reduce variance. Far more useful for signal monitoring and characterisation than a single FFT frame; the existing `FFT` block provides the transform kernel so this block adds only overlap buffering, window application, and accumulation.
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<value_type>> out`
- **Settings:** `fft_size`, `window_type`, `overlap` (fraction 0–1), `n_averages`
- **Processing:** `processBulk`

#### `SpectralSubtractor<T>` — P3
Estimates the noise floor from a silent reference interval and subtracts it in the frequency domain to reduce stationary noise. Useful in measurement and audio processing chains.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size`, `alpha` (spectral floor update rate), `reference_frames`
- **Processing:** `processBulk`

---

## Module: demod (new module suggested)

The framework currently has no demodulation blocks. The following cover the most common analog and digital cases.

#### `QuadratureDemod<T>` — P1 ✓ implemented
Implemented as `gr::blocks::math::QuadratureDemod<T>` in `blocks/math/include/gnuradio-4.0/math/QuadratureDemod.hpp`. Supports complex<float>, complex<double>; outputs the corresponding real scalar type.

#### `AmDemod<T>` — P2 ✓ implemented
Implemented as `gr::blocks::math::AmDemod<T>` in `blocks/math/include/gnuradio-4.0/math/AmDemod.hpp`. Computes `|in[n]|`; chain with DCBlocker for DSB-SC. Supports complex<float>, complex<double>; outputs the corresponding real scalar type.

#### `PLL<T>` — P1
Phase-Locked Loop for carrier recovery: tracks an input carrier and outputs a locked reference sinusoid. Essential for coherent demodulation of AM, PM, and narrowband FM signals.
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `loop_bandwidth`, `max_freq`, `min_freq`
- **Processing:** `processOne`

#### `CostasLoop<T>` — P2
Costas loop for joint carrier-frequency and carrier-phase recovery from BPSK, QPSK, or 8-PSK signals. Outputs the phase-corrected baseband symbols.
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `loop_bandwidth`, `order` (2 = BPSK, 4 = QPSK, 8 = 8PSK)
- **Processing:** `processOne`

#### `ClockRecoveryMM<T>` — P2
Mueller-Müller symbol timing recovery: adjusts the sampling instant to align with symbol centres using a feedback loop driven by the Mueller-Müller error signal.
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `omega` (samples per symbol), `loop_bandwidth`, `gain_mu`, `gain_omega`
- **Processing:** `processBulk` (variable output rate)

#### `SymbolSync<T>` — P2
Symbol timing synchronisation combining a Gardner timing error detector with a PI loop controller and a `FractionalDelayLine` interpolator. Adjusts the effective sampling phase continuously to align output samples with symbol centres. The Gardner detector is data-aided (decision-directed) and works for any linearly modulated signal (BPSK, QPSK, QAM) without a separate pilot. Requires `FractionalDelayLine` as a prerequisite.
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `sps` (samples per symbol), `loop_bandwidth`, `damping_factor`, `max_deviation`
- **State:** PI loop state, fractional delay accumulator
- **Processing:** `processBulk` (variable output rate — one sample per symbol)

---

## Module: coding (new module suggested)

#### `PackBits<T>` — P2
Packs `bits_per_chunk` LSBs of each input byte into a densely packed output byte stream. Standard pre-FEC or pre-modulator packing step.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `bits_per_chunk`
- **Processing:** `processBulk`

#### `UnpackBits<T>` — P2
Unpacks each input byte into `bits_per_chunk` output bytes, one bit per byte (LSB-justified). Counterpart to `PackBits`.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `bits_per_chunk`
- **Processing:** `processBulk`

#### `DifferentialEncoder<T>` — P2
Encodes the input bit stream differentially: `out[n] = out[n-1] XOR in[n]`. Removes phase ambiguity in BPSK/QPSK systems.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne`

#### `DifferentialDecoder<T>` — P2
Decodes differentially-encoded bits: `out[n] = in[n] XOR in[n-1]`. Counterpart to `DifferentialEncoder`.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne`

#### `Scrambler<T>` — P3
XORs the input with a pseudo-random binary sequence generated by a linear feedback shift register (LFSR). Used for data whitening to improve spectral flatness and aid clock recovery.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `mask`, `seed`, `len`
- **Processing:** `processOne`

#### `CrcCompute<T>` — P2
Computes a CRC-8, CRC-16, or CRC-32 checksum over a burst delimited by tags and appends (source mode) or verifies and strips (sink mode) the checksum bytes.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `poly`, `initial_value`, `mode` (append/verify)
- **Processing:** `processBulk` — `NoDefaultTagForwarding`

#### `GrayCodeEncoder` / `GrayCodeDecoder` — P3
Converts between natural binary and Gray code (reflected binary). Each output bit differs from the corresponding input by at most one bit transition per symbol, which reduces errors in ADC/DAC index decoding and FSK symbol mapping.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne` — stateless (`n ^ (n >> 1)` for encode; iterative XOR-fold for decode)

#### `ConvEncoder` — P3
Rate-1/2 or rate-1/3 convolutional encoder using a configurable constraint length and generator polynomials. Produces the standard encoded bit stream for use with a `ViterbiDecoder`. Stateless except for the shift register.
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `constraint_length`, `generator_polynomials`, `rate` (1/2 or 1/3)
- **State:** `uint32_t _shiftReg`
- **Processing:** `processOne`

#### `ViterbiDecoder` — P3
Maximum-likelihood decoder for rate-1/2 or rate-1/3 convolutional codes using the Viterbi algorithm. Accepts soft or hard decisions. The trellis is described by the same generator polynomials as `ConvEncoder`. Typical use: FEC on packet radio, satellite, and broadcast links.
- **Ports:** `PortIn<float> in` (soft LLRs) or `PortIn<uint8_t> in` (hard bits), `PortOut<uint8_t> out`
- **Settings:** `constraint_length`, `generator_polynomials`, `traceback_depth`
- **Processing:** `processBulk`
- **Note:** implement `ConvEncoder` first to generate test vectors.

---

## Module: ofdm (new module suggested)

#### `CyclicPrefixAdd<T>` — P2 ✓ implemented
Prepends a cyclic prefix of length `cp_length` to each block of `fft_size` samples. The cyclic prefix copies the last `cp_length` samples of the IFFT output to the front, converting linear convolution to circular convolution and eliminating inter-symbol interference in multipath channels. Required to make the existing `IFFT` block useful for OFDM transmission.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size`, `cp_length`
- **Processing:** `processBulk` — `Resampling<fft_size, fft_size + cp_length>`

#### `CyclicPrefixRemove<T>` — P2 ✓ implemented
Strips the `cp_length`-sample cyclic prefix from each received OFDM symbol before passing the remaining `fft_size` samples to the FFT block. Counterpart to `CyclicPrefixAdd`.
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size`, `cp_length`
- **Processing:** `processBulk` — `Resampling<fft_size + cp_length, fft_size>`

---

## Module: fileio

#### `WavFileSource<T>` — P2
Reads a WAV audio file (PCM, floating-point) and streams the samples as a typed output, exposing the sample rate from the file header as a tag or setting. The most common audio file format; needed for offline audio processing without format conversion.
- **Ports:** `PortOut<T> out`
- **Settings:** `file_name`, `repeat`
- **Processing:** `processBulk`

#### `WavFileSink<T>` — P2
Writes a sample stream to a WAV file at a given sample rate. Counterpart to `WavFileSource`.
- **Ports:** `PortIn<T> in`
- **Settings:** `file_name`, `sample_rate`, `bits_per_sample`
- **Processing:** `processBulk`

#### `SigMFSource<T>` — P2
Reads a SigMF recording (`.sigmf-data` + `.sigmf-meta` pair) and streams the samples, mapping SigMF annotations to GR4 tags. SigMF is the standard interchange format for SDR captures.
- **Ports:** `PortOut<T> out`
- **Settings:** `file_name` (base name without extension), `repeat`
- **Processing:** `processBulk`

#### `SigMFSink<T>` — P2
Records a sample stream to SigMF format, writing stream tags as SigMF annotations. Counterpart to `SigMFSource`.
- **Ports:** `PortIn<T> in`
- **Settings:** `file_name`, `sample_rate`, `datatype`, `author`, `description`
- **Processing:** `processBulk`

#### `CsvFileSink<T>` — P3
Writes one or more scalar streams to a CSV file with configurable column headers and separators. Useful for logging measurements and exporting data to analysis tools.
- **Ports:** `std::vector<PortIn<T>> in`
- **Settings:** `file_name`, `column_names`, `separator`, `timestamp_column`
- **Processing:** `processBulk`

#### `CsvFileSource<T>` — P3
Reads a CSV file and streams one column per output port. Counterpart to `CsvFileSink`.
- **Ports:** `std::vector<PortOut<T>> out`
- **Settings:** `file_name`, `column_indices`, `separator`, `skip_header`
- **Processing:** `processBulk`

---

## Module: network (new module suggested)

#### `UdpSource<T>` — P2
Receives raw sample data over UDP and outputs it as a typed stream. Low-latency network data source for distributed SDR or instrument interfacing.
- **Ports:** `PortOut<T> out`
- **Settings:** `bind_address`, `port`, `payload_size`, `eof_on_disconnect`
- **Processing:** `processBulk` (blocking)

#### `UdpSink<T>` — P2
Sends a sample stream over UDP. Counterpart to `UdpSource`.
- **Ports:** `PortIn<T> in`
- **Settings:** `address`, `port`, `payload_size`
- **Processing:** `processBulk`

#### `ZmqSource<T>` — P2
Receives sample data over a ZeroMQ socket (PUB/SUB or PUSH/PULL). Enables high-throughput inter-process and inter-host data transport with minimal coupling.
- **Ports:** `PortOut<T> out`
- **Settings:** `address`, `socket_type`, `timeout_ms`, `hwm`
- **Processing:** `processBulk`

#### `ZmqSink<T>` — P2
Publishes a sample stream over a ZeroMQ socket. Counterpart to `ZmqSource`.
- **Ports:** `PortIn<T> in`
- **Settings:** `address`, `socket_type`, `hwm`
- **Processing:** `processBulk`

---

## Module: audio (new module suggested)

#### `AudioSource<T>` — P2
Captures audio samples from a system audio device (PortAudio / PipeWire / ALSA). Provides a real-time audio input stream at configurable sample rate and buffer size.
- **Ports:** `std::vector<PortOut<T>> out` (one per channel)
- **Settings:** `device_name`, `sample_rate`, `buffer_size`, `n_channels`
- **Processing:** `processBulk` (blocking)

#### `AudioSink<T>` — P2
Plays a sample stream through a system audio device. Counterpart to `AudioSource`.
- **Ports:** `std::vector<PortIn<T>> in`
- **Settings:** `device_name`, `sample_rate`, `buffer_size`, `n_channels`
- **Processing:** `processBulk` (blocking)

---

## Module: electrical

#### `HarmonicAnalyser<T>` — P2
Measures the amplitude and phase of the fundamental and a configurable number of harmonics from a periodic signal using a synchronised DFT. Needed for THD (total harmonic distortion) and harmonic order analysis in power-quality measurement.
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<T>> out`
- **Settings:** `fundamental_frequency`, `n_harmonics`, `sample_rate`, `window_size`
- **Processing:** `processBulk`

#### `TotalHarmonicDistortion<T>` — P2
Computes Total Harmonic Distortion (THD and THD+N) from harmonic amplitude measurements. Accepts the output of `HarmonicAnalyser` or a spectrum.
- **Ports:** `PortIn<DataSet<T>> in`, `PortOut<T> thd`, `PortOut<T> thd_n`
- **Settings:** `n_harmonics`
- **Processing:** `processBulk`

#### `PhasorEstimator<T>` — P2
Estimates the complex phasor (amplitude and phase) of a near-sinusoidal signal at a known frequency using a Goertzel filter or a synchronous DFT over one cycle. More efficient than a full FFT for single-frequency measurement.
- **Ports:** `PortIn<T> in`, `PortOut<std::complex<T>> out`
- **Settings:** `frequency`, `sample_rate`, `window_size`
- **Processing:** `processBulk`

#### `GridFrequencyEstimator<T>` — P2
Estimates the instantaneous frequency of a power-grid waveform (45–65 Hz range) using a zero-crossing or PLL method optimised for 50/60 Hz signals with low SNR. More specialised and more robust than the general-purpose `FrequencyEstimatorTimeDomain` for grid applications.
- **Ports:** `PortIn<T> in`, `PortOut<T> frequency`
- **Settings:** `sample_rate`, `nominal_frequency` (50 or 60 Hz), `filter_bandwidth`
- **Processing:** `processBulk`

---

## New unconsidered blocks (not in original candidates list)

These blocks were identified as genuine gaps not covered by the original P1/P2/P3 list above.

### Module: math

#### `AgcBlock<T>` — P2 (new) ✓ implemented
Implemented as `gr::blocks::math::AgcBlock<T>` in `blocks/math/include/gnuradio-4.0/math/AgcBlock.hpp`. Uses per-sample gain adjustment with attack/decay rates; gain clamped to [min_gain, max_gain]. Supports float, double, complex<float>, complex<double>.

#### `PowerToDb<T>` / `DbToPower<T>` — P2 (new) ✓ implemented
Implemented as `gr::blocks::math::PowerToDb<T>` and `gr::blocks::math::DbToPower<T>` in `blocks/math/include/gnuradio-4.0/math/DbConvert.hpp`. Supports `amplitude_mode` flag (20·log10 vs 10·log10) and configurable `ref`. Types: float, double.

#### `Limiter<T>` — P2 (new) ✓ implemented
Implemented as `gr::blocks::math::Limiter<T>` in `blocks/math/include/gnuradio-4.0/math/Limiter.hpp`. For complex types, scales the magnitude to `limit` while preserving phase. Supports float, double, complex<float>, complex<double>.

### Module: filter

#### `CicDecimator<T>` / `CicInterpolator<T>` — P2 (new) ✓ implemented
Implemented as `gr::filter::CicDecimator<T>` and `gr::filter::CicInterpolator<T>` in `blocks/filter/include/gnuradio-4.0/filter/CicFilter.hpp`. Uses double-precision accumulators internally; output normalised by `1/R^N` (decimator) or `1/L^N` (interpolator). Types: float, double, int16_t, int32_t.

### Module: basic / testing

#### `EnergyDetector<T>` — P2 (new) ✓ implemented
Implemented as `gr::blocks::math::EnergyDetector<T>` in `blocks/math/include/gnuradio-4.0/math/EnergyDetector.hpp`. Pass-through block; publishes `tag_key={true/false}` on rising/falling threshold crossings. Supports float, double, complex<float>, complex<double>.

---

## Summary by priority

| Priority | Count | ✓ Implemented | Remaining |
|---|---|---|---|
| P1 | 12 | `Decimator`✓, `Interpolator`✓, `Keep1InN`✓, `MovingAverage`✓, `DCBlocker`✓, `HilbertTransform`✓, `QuadratureDemod`✓ (7/12) | `RationalResampler`, `StreamToVector`, `VectorToStream`, `IFFT`, `PLL` |
| P2 | 38+17 new | `Clamp`✓, `PhaseUnwrap`✓, `Conjugate`✓, `Differentiator`✓, `Accumulator`✓, `MovingRms`✓, `AmDemod`✓, `AgcBlock`✓, `PowerToDb/DbToPower`✓, `Limiter`✓, `CicDecimator/Interpolator`✓, `EnergyDetector`✓, `WienerFilter`✓, `Threshold`✓, `SchmittTrigger`✓, `InstantaneousFrequency`✓, `Goertzel`✓, `AutoCorrelation`✓, `Correlation`✓, `PeakDetector`✓ (20/55) | all others |
| P3 | 9+4 new | — | all |
| **Total** | **82** | **20 implemented** | **62 remaining** |

**New blocks added to this file (session 2+):**
- Unconsidered originally: `AgcBlock`, `PowerToDb/DbToPower`, `Limiter`, `CicDecimator/Interpolator`, `EnergyDetector` (P2)
- From further review (round 1): `SchmittTrigger`, `Goertzel`, `FractionalDelayLine`, `WienerFilter`✓ (P2); `GrayCodeEncoder/Decoder` (P3)
- From further review (round 2): `Throttle`, `Head`, `Skip`, `ChirpSource`, `AwgnChannel`, `HeaderPayloadDemux`, `AutoCorrelation`, `InstantaneousFrequency`, `BiquadFilter`, `SpectralEstimator`, `SymbolSync`, `CyclicPrefixAdd/Remove` (P2); `ConvEncoder`, `ViterbiDecoder` (P3)
- From further review (round 3): `KalmanFilter`, `SteadyStateKalman` (P2)
