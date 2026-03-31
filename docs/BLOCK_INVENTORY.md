# Block inventory

Complete listing of all `gr::Block<>` definitions in this repository, organised by module.
Generated from a two-pass review of the `add-blocks` branch.

**80 blocks** across 10 modules.

---

## Description quality issues

| Block | Issue |
|---|---|
| `DegreeToRadians` | `Doc<>` copy-paste bug: says "convert radians to degree" — should be "convert degree to radians" |
| `SettingsChangeRecorder` | Placeholder description: "some test doc documentation" |
| `builtin_multiply` | No `Doc<>` field |
| `builtin_counter` | No `Doc<>` field |
| `MathOpImpl` (4 scalar aliases) | No `Doc<>` field |
| `Delay` | No `Doc<>` field |

---

## Module: basic (`blocks/basic/`)

### `CommonBlocks.hpp`

#### `builtin_multiply<T>`
- **Description:** *(none)*
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `T factor`
- **Processing:** `processOne` — multiplies each sample by `factor`

#### `builtin_counter<T>`
- **Description:** *(none)*
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Processing:** `processOne` — counts samples passing through

---

### `ConverterBlocks.hpp`

#### `Convert<T, R>`
- **Description:** "basic block to perform a input to output data type conversion (w/o scaling)"
- **Ports:** `PortIn<T> in`, `PortOut<R> out`
- **Processing:** `processOne` — `static_cast<R>(x)`

#### `ScalingConvert<T, R>`
- **Description:** "basic block to perform a input to output data type conversion; performs scaling, i.e. `R output = R(input * scale)`"
- **Ports:** `PortIn<T> in`, `PortOut<R> out`
- **Settings:** `T scale = 1`
- **Processing:** `processOne`

#### `Abs<T>`
- **Description:** "calculate the absolute (magnitude) value of a complex or arithmetic input stream"
- **Ports:** `PortIn<T> in`, `PortOut<R> abs`
- **Processing:** `processOne` — `std::abs(x)`

#### `Real<T>`
- **Description:** "extract the real component of a complex input stream"
- **Ports:** `PortIn<T> in`, `PortOut<R> real`
- **Processing:** `processOne` — `std::real(x)`

#### `Imag<T>`
- **Description:** "extract the imaginary component of a complex input stream"
- **Ports:** `PortIn<T> in`, `PortOut<R> imag`
- **Processing:** `processOne` — `std::imag(x)`

#### `Arg<T>`
- **Description:** "calculate the argument (phase angle, [radians]) of a complex input stream"
- **Ports:** `PortIn<T> in`, `PortOut<R> arg`
- **Processing:** `processOne` — `std::arg(x)`

#### `RadiansToDegree<T>`
- **Description:** "convert radians to degree"
- **Ports:** `PortIn<T> rad`, `PortOut<R> deg`
- **Processing:** `processOne` — multiply by `180 / π`

#### `DegreeToRadians<T>`
- **Description:** "convert degree to radians" *(Doc<> has a copy-paste bug; says "convert radians to degree")*
- **Ports:** `PortIn<T> deg`, `PortOut<R> rad`
- **Processing:** `processOne` — multiply by `π / 180`

#### `ToRealImag<T>`
- **Description:** "decompose complex (or arithmetic) numbers into their real and imaginary components"
- **Ports:** `PortIn<T> in`, `PortOut<R> real`, `PortOut<R> imag`
- **Processing:** `processOne` — returns `{real, imag}` tuple

#### `RealImagToComplex<T>`
- **Description:** "compose complex (or arithmetic) numbers from their real and imaginary components"
- **Ports:** `PortIn<T> real`, `PortIn<T> imag`, `PortOut<R> out`
- **Processing:** `processOne` — constructs `R{real, imag}`

#### `ToMagPhase<T>`
- **Description:** "decompose complex (or arithmetic) numbers into their magnitude (abs) and phase (arg, [rad]) components"
- **Ports:** `PortIn<T> in`, `PortOut<R> mag`, `PortOut<R> phase`
- **Processing:** `processOne` — returns `{abs(x), arg(x)}`

#### `MagPhaseToComplex<T>`
- **Description:** "compose complex (or arithmetic) numbers from their abs and phase ([rad]) components"
- **Ports:** `PortIn<T> mag`, `PortIn<T> phase`, `PortOut<R> out`
- **Processing:** `processOne` — `polar(mag, phase)`

#### `ComplexToInterleaved<T, R>` — `Resampling<1, 2>`
- **Description:** "convert stream of complex to a stream of interleaved real/imaginary values of the specified type"
- **Ports:** `PortIn<T> in`, `PortOut<R> interleaved`
- **Processing:** `processBulk` — outputs 2× samples (re, im alternating)

#### `InterleavedToComplex<T, R>` — `Resampling<2, 1>`
- **Description:** "convert stream of interleaved values to a stream of complex numbers"
- **Ports:** `PortIn<T> interleaved`, `PortOut<R> out`
- **Processing:** `processBulk` — consumes 2× samples, outputs 1 complex per pair

---

### `ClockSource.hpp`

#### `ClockSource<T, ClockSourceType>`
- **Description:** "generates clock signals with specified timing intervals; supports optional zero-order hold and tag-based sequencing"
- **Ports:** `PortOut<T> out`
- **Settings:** `sample_rate`, `chunk_size`, `n_samples_max`, `tag_times`, `tag_values`, `repeat_period`, `do_zero_order_hold`, `use_internal_thread`
- **Processing:** `processBulk` (blocking sync)
- **Alias:** `DefaultClockSource = ClockSource<uint8_t, std::chrono::system_clock>`

---

### `SignalGenerator.hpp`

#### `SignalGenerator<T>`
- **Description:** "generates signal waveforms: sine, cosine, square, saw, triangle, constant, fast sine/cosine, and noise; driven by an optional external clock input"
- **Ports:** `PortIn<uint8_t, Optional> clk_in`, `PortOut<T> out`
- **Settings:** `sample_rate`, `chunk_size`, `signal_type`, `frequency`, `amplitude`, `offset`, `phase`, `seed`
- **Processing:** `processBulk`

---

### `FunctionGenerator.hpp`

#### `FunctionGenerator<T>`
- **Description:** "generates function waveforms and their combinations via tag-based sequencing (ramps, impulse responses, arbitrary multi-segment waveforms)"
- **Ports:** input trigger, `PortOut<T> out`
- **Settings:** signal type, timing triggers, segment parameters
- **Processing:** `processBulk`

---

### `Trigger.hpp`

#### `SchmittTrigger<T, Method>` — `NoDefaultTagForwarding`
- **Description:** "digital Schmitt trigger with configurable offset and threshold; optionally interpolates the precise trigger crossing time between samples"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `offset`, `threshold`, `trigger_name_rising_edge`, `trigger_name_falling_edge`, `sample_rate`, `forward_tag`, `trigger_name`, `trigger_time`, `trigger_offset`
- **Processing:** `processBulk` — publishes edge tags on transitions

---

### `SyncBlock.hpp`

#### `SyncBlock<T>` — `NoDefaultTagForwarding`
- **Description:** "synchronises data streams across multiple async inputs, aligning them by sample index or timestamp before forwarding to outputs"
- **Ports:** `std::vector<PortIn<T, Async>> inputs`, `std::vector<PortOut<T>> outputs`
- **Settings:** `n_ports`, `max_history_size`, `filter`, `tolerance`
- **Processing:** `processBulk`

---

### `Selector.hpp`

#### `Selector<T>` — `NoDefaultTagForwarding`
- **Description:** "basic multiplexing block routing arbitrary inputs to outputs; supports per-port mapping and optional back-pressure"
- **Ports:** dynamic `std::vector<PortIn<T>>` / `std::vector<PortOut<T>>`
- **Settings:** `n_inputs`, `n_outputs`, `map_in`, `map_out`, `sync_combined_ports`, `backPressure`
- **Processing:** `processBulk`

---

## Module: math (`blocks/math/`)

### `Math.hpp`

#### `MathOpImpl<T, op>` — scalar constant variants
- **Description:** *(none)*
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `T value` (constant operand applied to every sample)
- **Processing:** `processOne`
- **Registered as:** `AddConst<T>`, `SubtractConst<T>`, `MultiplyConst<T>`, `DivideConst<T>`

#### `MathOpMultiPortImpl<T, op>` — multi-input variants
- **Description:** "math block combining multiple inputs into a single output via a given binary operation"
- **Ports:** `std::vector<PortIn<T>> in`, `PortOut<T> out`
- **Settings:** `n_inputs`
- **Processing:** `processBulk`
- **Registered as:** `Add<T>`, `Subtract<T>`, `Multiply<T>`, `Divide<T>`

---

### `Rotator.hpp`

#### `Rotator<T>`
- **Description:** "shifts complex input samples by a given incremental phase every sample, performing continuous frequency translation"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `sample_rate`, `frequency_shift`, `phase_increment`, `initial_phase`
- **Processing:** `processOne` — multiplies by a rotating complex phasor

---

### `AgcBlock.hpp`

#### `AgcBlock<T>`
- **Description:** "automatic gain control: adjusts a scalar gain per sample to maintain target output RMS² power; attack/decay rates control response speed"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `target_power` (default 1), `attack_rate` (default 0.001), `decay_rate` (default 0.001), `max_gain` (default 65536), `min_gain` (default 1e-6)
- **State:** `value_type _gain`
- **Processing:** `processOne`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `AmDemod.hpp`

#### `AmDemod<T>`
- **Description:** "AM envelope detector: recovers the instantaneous amplitude of a complex baseband signal via `|in[n]|`; chain with DCBlocker for DSB-SC demodulation"
- **Ports:** `PortIn<T> in` (complex), `PortOut<value_type> out` (real scalar)
- **Processing:** `processOne` — stateless
- **Types:** `std::complex<float>`, `std::complex<double>`

---

### `Clamp.hpp`

#### `Clamp<T>`
- **Description:** "clips each sample to [min, max]; for complex types, real and imaginary parts are clamped independently"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `min` (default -1), `max` (default 1)
- **Processing:** `processOne` — stateless
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `EnergyDetector.hpp`

#### `EnergyDetector<T>`
- **Description:** "moving-energy threshold detector: publishes a tag on rising/falling threshold crossings; samples pass through unchanged"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size` (default 32), `threshold` (linear |x|² energy), `hysteresis` (default 0), `tag_key` (default "energy_detect")
- **State:** `HistoryBuffer<value_type> _powerHistory`, `value_type _runningEnergy`, `std::size_t _filledCount`, `bool _detected`
- **Processing:** `processOne` — publishes `tag_key={true/false}` on edge crossings
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `DbConvert.hpp`

#### `PowerToDb<T>`
- **Description:** "converts linear power to dB: `10 * log10(in / ref)`; set `amplitude_mode = true` for `20 * log10`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `ref` (default 1), `amplitude_mode` (default false)
- **Processing:** `processOne` — stateless
- **Types:** `float`, `double`

#### `DbToPower<T>`
- **Description:** "converts dB back to linear power: `ref * 10^(in / 10)`; set `amplitude_mode = true` for `10^(in / 20)`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `ref` (default 1), `amplitude_mode` (default false)
- **Processing:** `processOne` — stateless
- **Types:** `float`, `double`

---

### `Limiter.hpp`

#### `Limiter<T>`
- **Description:** "symmetric hard amplitude limiter: real types use `clamp(x, -limit, limit)`; complex types scale magnitude to `limit` while preserving phase"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `limit` (default 1)
- **Processing:** `processOne` — stateless
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `Accumulator.hpp`

#### `Accumulator<T>`
- **Description:** "running sum (numerical integration): `out[n] = out[n-1] + in[n]`; resets to zero on a tagged sample when `reset_tag_key` is set"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `reset_tag_key` (empty = never reset)
- **State:** `T _sum`
- **Processing:** `processOne`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `MovingAverage.hpp`

#### `MovingAverage<T>`
- **Description:** "causal moving average filter over a configurable window of samples using an O(1)-per-sample running-sum implementation; warm-up phase averages only available samples"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `length` (default 16) — window size in samples
- **State:** `HistoryBuffer<T> _history`, `T _runningSum`, `std::size_t _filledCount`
- **Processing:** `processOne` — `_runningSum * (1 / min(_filledCount, len))`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `MovingRms.hpp`

#### `MovingRms<T>`
- **Description:** "causal root-mean-square estimator over a sliding window; computes `sqrt(mean(|x|²))` using an O(1)-per-sample running-sum; output is always real `value_type`"
- **Ports:** `PortIn<T> in`, `PortOut<value_type> out` (real scalar)
- **Settings:** `length` (default 16) — window size in samples
- **State:** `HistoryBuffer<value_type> _powerHistory`, `value_type _runningPower`, `std::size_t _filledCount`
- **Processing:** `processOne`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `QuadratureDemod.hpp`

#### `QuadratureDemod<T>`
- **Description:** "FM/PM demodulator that recovers instantaneous frequency from a complex baseband signal by computing `gain * arg(x[n] * conj(x[n-1]))`"
- **Ports:** `PortIn<T> in` (complex), `PortOut<value_type> out` (real scalar)
- **Settings:** `gain` (default 1) — scales phase-difference output; set to `sample_rate / (2π · max_deviation)` for FM
- **State:** `T _prev`
- **Processing:** `processOne`
- **Types:** `std::complex<float>`, `std::complex<double>`

---

### `PhaseUnwrap.hpp`

#### `PhaseUnwrap<T>`
- **Description:** "removes 2π discontinuities from a wrapped phase signal by tracking an accumulated phase offset; essential post-processing step after `Arg<T>` or `QuadratureDemod`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **State:** `T _prev`, `T _offset`
- **Processing:** `processOne`
- **Types:** `float`, `double`

---

### `Conjugate.hpp`

#### `Conjugate<T>`
- **Description:** "computes the complex conjugate of each input sample: `out[n] = conj(in[n])`; trivially stateless"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Processing:** `processOne` — stateless
- **Types:** `std::complex<float>`, `std::complex<double>`

---

### `Differentiator.hpp`

#### `Differentiator<T>`
- **Description:** "first-order backward difference: `out[n] = in[n] − in[n−1]`; first output equals `in[0]` (prev initialised to zero)"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **State:** `T _prev`
- **Processing:** `processOne`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `ExpressionBlocks.hpp`

#### `ExpressionSISO<T>`
- **Description:** "Single-Input-Single-Output (SISO) expression evaluator using ExprTK; evaluates a user-supplied formula per sample"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `expr_string`, `param_a`, `param_b`, `param_c`
- **Processing:** `processOne`

#### `ExpressionDISO<T>`
- **Description:** "Dual-Input-Single-Output (DISO) expression evaluator using ExprTK"
- **Ports:** `PortIn<T> in0`, `PortIn<T> in1`, `PortOut<T> out`
- **Settings:** `expr_string`, `param_a`, `param_b`, `param_c`
- **Processing:** `processOne`

#### `ExpressionBulk<T>`
- **Description:** "bulk array expression evaluator using ExprTK; operates on entire input spans for higher throughput"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `expr_string`
- **Processing:** `processBulk`

---

## Module: electrical (`blocks/electrical/`)

### `PowerEstimators.hpp`

#### `PowerMetrics<T, nPhases>` — `Resampling<100, 1>`
- **Description:** "computes per-phase active power (P), reactive power (Q), apparent power (S), RMS voltage, and RMS current from voltage and current input streams"
- **Ports:** `std::vector<PortIn<T>> U` (voltage), `std::vector<PortIn<T>> I` (current); output ports for `P`, `Q`, `S`, `U_rms`, `I_rms`
- **Settings:** `sample_rate`, `high_pass`, `low_pass`, `decimate`
- **Processing:** `processBulk`
- **Aliases:** `SinglePhasePowerMetrics`, `ThreePhasePowerMetrics`

#### `PowerFactor<T, nPhases>`
- **Description:** "calculates the power factor and phase angle for each phase from P, Q, and S inputs"
- **Ports:** multiple inputs (`P`, `Q`, `S`), outputs for power factor and phase angle per phase
- **Processing:** `processBulk`
- **Aliases:** `SinglePhasePowerFactorCalculator`, `ThreePhasePowerFactorCalculator`

#### `SystemUnbalance<T, nPhases>`
- **Description:** "computes total active power and the system unbalance ratio across multiple phases"
- **Ports:** multi-phase P/Q/S inputs; outputs for unbalance ratio and total power
- **Processing:** `processBulk`

---

## Module: fileio (`blocks/fileio/`)

### `BasicFileIo.hpp`

#### `BasicFileSink<T>`
- **Description:** "a sink block for writing a stream to a binary file; supports overwrite, append, and multi-file (rolling) modes"
- **Ports:** `PortIn<T> in`
- **Settings:** `file_name`, `mode` (overwrite/append/multi), `max_bytes_per_file`
- **Processing:** `processBulk` — writes raw binary data via the FileIo writer

#### `BasicFileSource<T>`
- **Description:** "a source block for reading a binary file and outputting the data as a typed sample stream"
- **Ports:** `PortOut<T> out`
- **Settings:** `file_name`, `repeat`
- **Processing:** `processBulk` — reads raw binary data via the FileIo reader

---

## Module: fourier (`blocks/fourier/`)

### `fft.hpp`

#### `FFT<T, U, FourierAlgorithm>` — `Resampling<1024, 1>`
- **Description:** "performs a (Fast) Fourier Transform on the input; outputs a DataSet containing the spectrum with a configurable window function and FFT size"
- **Ports:** `PortIn<T> in` (real or complex), `PortOut<DataSet<U>> out`
- **Settings:** `fft_size`, `window_type`, `sample_rate`, `output_in_db`
- **Processing:** `processBulk`

---

## Module: filter (`blocks/filter/`)

#### `fir_filter<T>`
- **Description:** "Finite Impulse Response (FIR) filter; applies a user-supplied set of tap coefficients via direct-form convolution"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `coefficients`
- **Processing:** `processOne` / `processBulk`

#### `iir_filter<T>`
- **Description:** "Infinite Impulse Response (IIR) filter; applies biquad or direct-form II transposed sections from user-supplied coefficients"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `b_coefficients`, `a_coefficients`
- **Processing:** `processOne`

#### `BasicFilterProto<T>`
- **Description:** "basic digital filter supporting both FIR and IIR modes; serves as a prototype/reference implementation"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** filter type, coefficients
- **Processing:** `processOne`

#### `FrequencyEstimatorTimeDomain<T>`
- **Description:** "estimates the dominant frequency of a signal using zero-crossing analysis in the time domain"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `sample_rate`, `f_min`, `f_expected`, `f_max`, `n_periods`, `epsilon`
- **Processing:** `processOne` / `processBulk` (resampling variant)

#### `FrequencyEstimatorFrequencyDomain<T>`
- **Description:** "estimates the dominant frequency by locating the peak bin in the FFT of an input window"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `sample_rate`, `fft_size`, window parameters
- **Processing:** `processBulk`

#### `SvdDenoiser<T>`
- **Description:** "SVD-based signal denoiser: constructs a Hankel matrix from the signal, decomposes it via SVD, thresholds singular values, and reconstructs the denoised signal"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size`, `hankel_rows`, `max_rank`, `relative_threshold`, `absolute_threshold`, `energy_fraction`, `hop_fraction`
- **Processing:** `processOne`

#### `SavitzkyGolayFilter<T>`
- **Description:** "Savitzky-Golay streaming filter; fits a low-degree polynomial over a moving window to smooth the signal while preserving peak shape"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size`, `polynomial_order`, `derivative`
- **Processing:** `processBulk`

#### `SavitzkyGolayDataSetFilter<T>`
- **Description:** "zero-phase Savitzky-Golay filter operating on DataSet objects; applies the filter forward and backward to eliminate phase distortion"
- **Ports:** `PortIn<DataSet<T>> in`, `PortOut<DataSet<T>> out`
- **Settings:** `window_size`, `polynomial_order`
- **Processing:** `processBulk`

#### `DCBlocker<T>`
- **Description:** "removes the DC component using a causal moving-average estimator: `out = input − mean(last length samples)`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `length` (default 32) — window size in samples
- **State:** `HistoryBuffer<T> _history`, `T _runningSum`, `std::size_t _filledCount`
- **Processing:** `processOne`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

#### `HilbertTransform<T>`
- **Description:** "produces the analytic signal from a real input: `out[n] = in[n−M] + j·H{in}[n]` using an odd-symmetric Hamming-windowed Type-III FIR filter; the real output is delayed by `(n_taps−1)/2` samples to align with the FIR output"
- **Ports:** `PortIn<T> in`, `PortOut<std::complex<T>> out`
- **Settings:** `n_taps` (default 63, must be odd)
- **State:** `std::vector<T> _taps`, `HistoryBuffer<T> _history`, `std::size_t _center`
- **Processing:** `processOne`
- **Types:** `float`, `double`

#### `CicDecimator<T>` — `Resampling<1UZ, 1UZ, false>`
- **Description:** "CIC decimation filter: N integrator stages → R:1 downsample → N comb stages; output normalised by `1/R^N`; multiplier-free for very high rate reductions"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `decimation` (R, default 8), `n_stages` (N, default 5), `differential_delay` (M, default 1)
- **State:** `std::vector<double> _integrators`, `std::vector<double> _combs`
- **Processing:** `processBulk` — dynamic `input_chunk_size = R`
- **Types:** `float`, `double`, `std::int16_t`, `std::int32_t`

#### `CicInterpolator<T>` — `Resampling<1UZ, 1UZ, false>`
- **Description:** "CIC interpolation filter: N comb stages → 1:L upsample → N integrator stages; output normalised by `1/L^N`; multiplier-free for very high rate increases"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `interpolation` (L, default 8), `n_stages` (N, default 5), `differential_delay` (M, default 1)
- **State:** `std::vector<double> _combs`, `std::vector<double> _integrators`
- **Processing:** `processBulk` — dynamic `output_chunk_size = L`
- **Types:** `float`, `double`, `std::int16_t`, `std::int32_t`

---

#### `Interpolator<T>` — `Resampling<1UZ, 1UZ, false>`
- **Description:** "zero-insertion upsampler: each input sample is followed by `interp − 1` zero-valued samples; no anti-imaging filter — chain with an FIR low-pass for proper interpolation"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `interp` (default 1) — upsampling factor; sets `output_chunk_size` dynamically
- **Processing:** `processBulk`
- **Types:** all numeric types including `UncertainValue<float/double>`
- **Note:** counterpart to the existing `Decimator<T>`

#### `Repeat<T>` — `Resampling<1UZ, 1UZ, false>`
- **Description:** "zero-order-hold (sample-and-hold) upsampler: repeats each input sample `repeat` times on the output; unlike Interpolator, no zeros are inserted"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `repeat` (default 1) — hold factor; sets `output_chunk_size` dynamically
- **Processing:** `processBulk`
- **Types:** all numeric types

#### `WienerFilter<T>`
- **Description:** "optimal MMSE FIR filter derived from the Wiener-Hopf normal equations; trains on paired `(in, desired)` streams for `training_length` samples, solves R_xx·h = r_xd once via Gaussian elimination, then applies frozen taps; output is zero during the training phase"
- **Ports:** `PortIn<T> in`, `PortIn<T> desired`, `PortOut<T> out`
- **Settings:** `n_taps` (default 16), `training_length` (default 256), `regularisation` (default 1e-6)
- **State:** `HistoryBuffer<T> _xHistory`, `std::vector<T> _Rxx` (N×N), `std::vector<T> _rxd` (N), `std::vector<T> _taps` (N)
- **Processing:** `processBulk`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

## Module: http (`blocks/http/`)

### `HttpBlock.hpp`

#### `HttpSource`
- **Description:** "reads data from an HTTP endpoint in GET or SUBSCRIBE (streaming) mode; emits parsed JSON/map values"
- **Ports:** `PortOut<pmt::Value::Map> out`
- **Settings:** `url`, `type` (GET/SUBSCRIBE), `chunk_bytes`
- **Processing:** async work function

#### `HttpSink`
- **Description:** "sends incoming byte stream to an HTTP endpoint via POST"
- **Ports:** `PortIn<uint8_t> in`
- **Settings:** `url`, `content_type`
- **Processing:** `processBulk`

---

## Module: soapy (`blocks/soapy/`)

### `Soapy.hpp`

#### `SoapyBlock<T, nPorts>`
- **Description:** "interfaces with SDR hardware via the SoapySDR library; configures RX channels, centre frequency, bandwidth, gain, and sample rate"
- **Ports:** `PortOut<T> out` (1, 2, or dynamic channel variants)
- **Settings:** `device`, `device_parameter`, `sample_rate`, `rx_channels`, `rx_antennae`, `rx_center_frequency`, `rx_bandwidth`, `rx_gains`, `max_chunk_size`, `max_time_out_us`, `max_overflow_count`
- **Processing:** `processBulk`
- **Aliases:** `SoapySimpleSource`, `SoapyDualSimpleSource`

---

## Module: testing (`blocks/testing/`)

### `NullSources.hpp`

#### `NullSource<T>`
- **Description:** "a source block that emits the default-constructed (zero) value of `T` continuously"
- **Ports:** `PortOut<T> out`
- **Processing:** `processOne`

#### `ConstantSource<T>`
- **Description:** "a source block that emits a constant configurable value; stops after `n_samples_max` samples if set"
- **Ports:** `PortOut<T> out`
- **Settings:** `default_value`, `n_samples_max`
- **Processing:** `processOne`

#### `SlowSource<T>`
- **Description:** "a source block that emits a constant value at a throttled rate (one chunk every N milliseconds); useful for simulating slow sensors"
- **Ports:** `PortOut<T> out`
- **Settings:** `default_value`, `delay` (ms)
- **Processing:** `processBulk`

#### `CountingSource<T>`
- **Description:** "a source block that emits a monotonically increasing integer sequence; stops after a configurable count"
- **Ports:** `PortOut<T> out`
- **Settings:** `n_samples_max`, starting value
- **Processing:** `processOne`

#### `Copy<T>`
- **Description:** "passes input samples directly to output without modification; useful as a no-op placeholder or for testing graph wiring"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Processing:** `processOne`

#### `HeadBlock<T>`
- **Description:** "limits the total number of samples forwarded to output then signals the graph to stop; the stream equivalent of Unix `head`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `n_samples_max`
- **Processing:** `processOne`

#### `NullSink<T>`
- **Description:** "a sink block that consumes and discards all incoming samples; useful for terminating unused outputs"
- **Ports:** `PortIn<T> in`
- **Processing:** `processBulk`

#### `CountingSink<T>`
- **Description:** "a sink block that counts the total number of samples received; useful for verifying throughput in tests"
- **Ports:** `PortIn<T> in`
- **Settings:** expected count / assertion
- **Processing:** `processOne` / `processBulk`

#### `Delay<T>`
- **Description:** *(none)* — introduces a configurable sample delay into the data stream using an internal ring buffer
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `delay_ms`
- **Processing:** `processBulk`

#### `SettingsChangeRecorder<T>`
- **Description:** *(placeholder: "some test doc documentation")* — records settings-change events for testing `settingsChanged` callbacks
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `scaling_factor`, `context`, `n_samples_max`, `sample_rate`, many additional test fields
- **Processing:** `processOne`

#### `PerformanceMonitor<T>`
- **Description:** "tracks and reports throughput (samples/s) and memory usage; optionally writes results to a CSV file"
- **Ports:** `PortIn<T> in`, `PortOut<double, Optional> outRes`, `PortOut<double, Optional> outRate`
- **Settings:** `evaluate_perf_rate`, `publish_rate`, `output_csv_file_path`
- **Processing:** `processBulk`

#### `ImChartMonitor<T, drawAsynchronously>`
- **Description:** "displays real-time signal data in a terminal ImChart plot; supports sample history, tag annotations, and configurable plot dimensions"
- **Ports:** `PortIn<T> in`
- **Settings:** `sample_rate`, `signal_name`, `signal_index`, `n_history`, `n_tag_history`, `reset_view`, `chart_width`, `chart_height`
- **Processing:** `processBulk`
- **Alias:** `ConsoleDebugSink<T>` = `ImChartMonitor<T, false>`

---

## Module: timing (`blocks/timing/`)

### `GpsSource.hpp`

#### `GpsSource`
- **Description:** "GPS/GNSS serial timing source; reads NMEA sentences from a serial device, parses UTC time, and publishes PPS-aligned timing tags"
- **Ports:** `PortOut<uint8_t> out`
- **Settings:** `device_path`, `device_name`, `trigger_name`, `context`, `emit_mode`, `sample_rate`, `emit_meta_info`, `emit_device_info`, `baud_rate`, `update_rate_ms`
- **Processing:** custom `work()` with PPS publishing

### `PpsSource.hpp`

#### `PpsSource`
- **Description:** "Linux-kernel PPS (Pulse-Per-Second) timing source; reads hardware PPS events via the Linux PPS API and publishes precise 1-second timing tags"
- **Ports:** timing output
- **Settings:** `clock_mode`, `pps_device`, `ptp_device`
- **Processing:** custom timing-driven work function

---

## Summary

| Module | Count |
|---|---|
| basic | 22 |
| math | 11 |
| electrical | 3 |
| fileio | 2 |
| fourier | 1 |
| filter | 11 |
| http | 2 |
| soapy | 1 |
| testing | 12 |
| timing | 2 |
| **Total** | **68** |

**Recently added:** `Accumulator`, `AgcBlock`, `AmDemod`, `Clamp`, `DbConvert` (PowerToDb + DbToPower), `EnergyDetector`, `Limiter`, `MovingAverage`, `MovingRms`, `QuadratureDemod`, `PhaseUnwrap`, `Conjugate`, `Differentiator` (math); `CicDecimator`, `CicInterpolator`, `DCBlocker`, `HilbertTransform`, `Interpolator`, `Repeat`, `WienerFilter` (filter).
