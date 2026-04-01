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

### `Head.hpp`

#### `Head<T>`
- **Description:** "forwards the first `n_samples` samples then calls `requestStop()`; the stream equivalent of Unix `head`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `n_samples` (default 1024)
- **Processing:** `processBulk`
- **Types:** all numeric types

### `Skip.hpp`

#### `Skip<T>`
- **Description:** "discards the first `n_samples` samples then passes all subsequent samples through unchanged"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `n_samples` (default 0)
- **Processing:** `processBulk`
- **Types:** all numeric types

### `ChirpSource.hpp`

#### `ChirpSource<T>`
- **Description:** "linear FM chirp source: sweeps from `f_start` to `f_end` over `sweep_time` seconds then resets"
- **Ports:** `PortOut<T> out`
- **Settings:** `f_start`, `f_end`, `sweep_time`, `sample_rate` (default 1.0)
- **Processing:** `processBulk` — source block
- **Types:** `float`, `double`

### `AwgnChannel.hpp`

#### `AwgnChannel<T>`
- **Description:** "additive white Gaussian noise channel; adds normally-distributed noise with variance `noise_power` to every sample"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `noise_power` (default 0.01)
- **Processing:** `processOne`
- **Types:** `float`, `double`

### `StreamTagger.hpp`

#### `StreamTagger<T>`
- **Description:** "injects a tag every `interval` samples containing the running sample count; `interval = 0` disables tagging"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `interval` (default 1024), `key` (tag key name, default "stream_tagger")
- **Processing:** `processOne`
- **Types:** all numeric types

### `TagGate.hpp`

#### `TagGate<T>`
- **Description:** "opens or closes the signal path based on a named boolean tag; samples are zeroed (not dropped) when the gate is closed"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `key` (tag key to watch, default "gate"), `initially_open` (default false)
- **Processing:** `processOne`
- **Types:** all numeric types

### `TagDebugSink.hpp`

#### `TagDebugSink<T>`
- **Description:** "sink block that logs all received tags to stdout for debugging; stores them for programmatic access in tests"
- **Ports:** `PortIn<T> in`
- **Settings:** `log_to_stdout` (default true)
- **Processing:** `processBulk`
- **Types:** all numeric types

### `WindowApply.hpp`

#### `WindowApply<T>`
- **Description:** "multiplies each block of `fft_size` input samples by the specified window function from `gr::algorithm::window`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size` (default 1024), `window_type` (default "hann")
- **Processing:** `processBulk` — `input_chunk_size = output_chunk_size = fft_size`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

### `StreamMux.hpp`

#### `StreamMux<T>`
- **Description:** "round-robin interleave from `n_inputs` dynamic input ports; outputs `chunk_size` samples from each port in sequence"
- **Ports:** `std::vector<PortIn<T>> inputs`, `PortOut<T> out`
- **Settings:** `n_inputs` (default 2), `chunk_size` (default 1)
- **Processing:** `processBulk` — dynamic `Resampling<1,1,false>`
- **Types:** all numeric types

### `StreamDemux.hpp`

#### `StreamDemux<T>`
- **Description:** "round-robin split to `n_outputs` dynamic output ports; forwards `chunk_size` input samples to each port in sequence"
- **Ports:** `PortIn<T> in`, `std::vector<PortOut<T>> outputs`
- **Settings:** `n_outputs` (default 2), `chunk_size` (default 1)
- **Processing:** `processBulk` — dynamic `Resampling<1,1,false>`
- **Types:** all numeric types

### `KeepMInN.hpp`

#### `KeepMInN<T>`
- **Description:** "forwards the first `m` of every `n` input samples and discards the remaining `n - m`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `m` (default 1), `n` (default 2)
- **Processing:** `processBulk` — dynamic `Resampling<1,1,false>`
- **Types:** all numeric types

### `HeaderPayloadDemux.hpp`

#### `HeaderPayloadDemux<T>`
- **Description:** "tag-triggered burst demultiplexer; on detecting `trigger_tag_key`, routes `header_length` samples to `header` then `payload_length` samples to `payload`; both outputs emit `T{}` when inactive"
- **Ports:** `PortIn<T> in`, `PortOut<T> header`, `PortOut<T> payload`
- **Settings:** `header_length` (default 8), `payload_length` (default 64), `trigger_tag_key` (default "trigger")
- **State:** `State _state` (Idle/Header/Payload), `std::size_t _count`
- **Processing:** `processOne` — returns `std::tuple<T, T>` for the two output ports
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

### `StreamToVector.hpp`

#### `StreamToVector<T>`
- **Description:** "groups `vlen` consecutive input samples into a single DataSet<T> output item"
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<T>> out`
- **Settings:** `vlen` (default 1024)
- **Processing:** `processBulk` — `input_chunk_size = vlen`, `output_chunk_size = 1`
- **Types:** all numeric types

### `VectorToStream.hpp`

#### `VectorToStream<T>`
- **Description:** "unpacks the first signal of each input DataSet<T> into a flat sample stream of length `vlen`"
- **Ports:** `PortIn<DataSet<T>> in`, `PortOut<T> out`
- **Settings:** `vlen` (default 1024)
- **Processing:** `processBulk` — `input_chunk_size = 1`, `output_chunk_size = vlen`
- **Types:** all numeric types

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

### `Correlation.hpp`

#### `Correlation<T>`
- **Description:** "sliding cross-correlation between two input streams: `out[n] = (1/W) · Σ_{k=0}^{W-1} signal[n-k] · conj(reference[n-k])`; O(1)-per-sample running-sum implementation"
- **Ports:** `PortIn<T> signal`, `PortIn<T> reference`, `PortOut<T> out`
- **Settings:** `window_size` (default 32)
- **State:** `HistoryBuffer<T> _sigHistory`, `HistoryBuffer<T> _refHistory`, `T _runningSum`, `std::size_t _filledCount`
- **Processing:** `processBulk`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `PeakDetector.hpp`

#### `PeakDetector<T>`
- **Description:** "detects local maxima using a look-ahead window; peak samples are passed through, non-peaks are zeroed; a tag is published at each peak; introduces `look_ahead` samples of latency"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `look_ahead` (default 8), `min_peak_height` (default 0), `min_peak_distance` (default 1), `tag_key` (default "peak")
- **State:** `std::vector<T> _buf` (circular look-ahead buffer), `_head`, `_filled`, `_samplesSincePeak`
- **Processing:** `processBulk` — `Resampling<1,1,false>`
- **Types:** `float`, `double`

---

### `AutoCorrelation.hpp`

#### `AutoCorrelation<T>`
- **Description:** "biased autocorrelation estimator: r[k] = (1/W)·Σ x[n]·conj(x[n-k]) for lags k = 0..max_lag; consumes `window_size` inputs per call and emits `max_lag+1` outputs"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size` (default 64), `max_lag` (default 16)
- **Processing:** `processBulk` — `Resampling<1,1,false>`; `input_chunk_size = window_size`, `output_chunk_size = max_lag+1`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

---

### `Goertzel.hpp`

#### `Goertzel<T>`
- **Description:** "single-frequency DFT magnitude via Goertzel's second-order IIR; emits one magnitude value per `block_size` input samples; O(N) per block with far lower overhead than a full FFT when only one frequency bin is needed"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `target_frequency` (default 1000 Hz), `sample_rate` (default 8000 Hz), `block_size` (default 128)
- **Processing:** `processBulk` — `Resampling<1,1,false>`; `input_chunk_size = block_size`, `output_chunk_size = 1`
- **Types:** `float`, `double`

---

### `InstantaneousFrequency.hpp`

#### `InstantaneousFrequency<T>`
- **Description:** "computes the instantaneous frequency of a complex analytic signal per sample: `f[n] = arg(x[n]·conj(x[n-1])) · sample_rate / (2π)`; first output is zero"
- **Ports:** `PortIn<T> in` (complex), `PortOut<value_type> out` (real scalar)
- **Settings:** `sample_rate` (default 1; set to 1 for normalised frequency in cycles/sample)
- **State:** `T _prev`
- **Processing:** `processOne`
- **Types:** `std::complex<float>`, `std::complex<double>`

---

### `SchmittTrigger.hpp`

#### `SchmittTrigger<T>`
- **Description:** "hysteretic comparator: output transitions to `high_value` only when input rises above `upper_threshold`, and back to `low_value` only when input falls below `lower_threshold`; prevents chatter on noisy signals"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `upper_threshold` (default 0.5), `lower_threshold` (default -0.5), `high_value` (default 1), `low_value` (default 0)
- **State:** `bool _state`
- **Processing:** `processOne`
- **Types:** `float`, `double`

---

### `Threshold.hpp`

#### `Threshold<T>`
- **Description:** "hard threshold comparator: emits `high_value` when input exceeds `threshold`, otherwise `low_value`; no hysteresis — see `SchmittTrigger` for noise-immune switching"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `threshold` (default 0), `high_value` (default 1), `low_value` (default 0)
- **Processing:** `processOne` — stateless
- **Types:** `float`, `double`

---

### `Histogram.hpp`

#### `Histogram<T>`
- **Description:** "amplitude histogram estimator; accumulates `accumulate_n` samples into `n_bins` uniform bins over [`min_value`, `max_value`]; emits one DataSet<T> per window with bin centres as axis and normalised counts as signal"
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<T>> out`
- **Settings:** `n_bins` (default 64), `min_value` (default -1), `max_value` (default 1), `accumulate_n` (default 1024)
- **State:** `std::vector<std::size_t> _counts`
- **Processing:** `processBulk` — `input_chunk_size = accumulate_n`, `output_chunk_size = 1`; out-of-range samples are ignored
- **Types:** `float`, `double`

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

#### `PhasorEstimator<T>`
- **Description:** "single-frequency phasor estimator using the Goertzel algorithm; outputs one complex phasor per `block_size` input samples"
- **Ports:** `PortIn<T> in`, `PortOut<std::complex<T>> out`
- **Settings:** `frequency` (default 50.0), `sample_rate` (default 1000.0), `block_size` (default 100)
- **Processing:** `processBulk` — `Resampling<1,1,false>` with `input_chunk_size = block_size`, `output_chunk_size = 1`
- **Types:** `float`, `double`

#### `HarmonicAnalyser<T>`
- **Description:** "analyses harmonics at the fundamental and `n_harmonics` multiples using Goertzel; outputs a `DataSet<T>` with amplitudes and phases"
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<T>> out`
- **Settings:** `fundamental` (default 50.0), `sample_rate` (default 10000.0), `n_harmonics` (default 5), `block_size` (default 200)
- **Processing:** `processBulk` — `input_chunk_size = block_size`, `output_chunk_size = 1`
- **Types:** `float`, `double`

#### `TotalHarmonicDistortion<T>`
- **Description:** "computes THD from a `DataSet<T>` of harmonic amplitudes produced by `HarmonicAnalyser`; THD = sqrt(V2²+…+Vn²) / V1"
- **Ports:** `PortIn<DataSet<T>> in`, `PortOut<T> out`
- **Processing:** `processOne`
- **Types:** `float`, `double`

#### `GridFrequencyEstimator<T>`
- **Description:** "estimates mains frequency from positive-going zero crossings; outputs one frequency estimate per crossing pair"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `sample_rate` (default 10000.0), `nominal_frequency` (default 50.0)
- **Processing:** `processBulk` — variable output rate
- **Types:** `float`, `double`

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

### `ifft.hpp`

#### `IFFT<T>`
- **Description:** "inverse discrete Fourier Transform using the conjugate-symmetry trick: `IFFT(X) = conj(FFT(conj(X))) / N`"
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `fft_size` (default 1024)
- **Processing:** `processBulk` — dynamic `Resampling<1,1,false>` with `input_chunk_size = output_chunk_size = fft_size`
- **Types:** `float`, `double`

### `SpectralEstimator.hpp`

#### `SpectralEstimator<T>`
- **Description:** "Bartlett power spectral density estimate: averages `n_averages` non-overlapping windowed FFT frames; outputs a `DataSet<T>` with frequency axis and PSD values"
- **Ports:** `PortIn<T> in`, `PortOut<DataSet<T>> out`
- **Settings:** `fft_size` (default 1024), `n_averages` (default 8), `sample_rate` (default 1.0), `window_type` (default "hann")
- **Processing:** `processBulk` — `input_chunk_size = fft_size * n_averages`, `output_chunk_size = 1`
- **Types:** `float`, `double`

### `SpectralSubtractor.hpp`

#### `SpectralSubtractor<T>`
- **Description:** "frequency-domain noise reduction via spectral subtraction; estimates the noise floor from the first `reference_frames` input frames, then subtracts `alpha` × noise RMS from each bin magnitude in subsequent frames"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size` (default 512), `alpha` (default 1.0, over-subtraction factor), `reference_frames` (default 10)
- **Processing:** `processBulk` — `input_chunk_size = output_chunk_size = fft_size`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

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

#### `BiquadFilter<T>`
- **Description:** "second-order IIR filter (biquad); implements a direct-form II transposed section using user-supplied `b0,b1,b2,a1,a2` coefficients"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `b0`, `b1`, `b2`, `a1`, `a2`
- **State:** `T _w1`, `T _w2`
- **Processing:** `processOne`
- **Types:** `float`, `double`

#### `FractionalDelayLine<T>`
- **Description:** "sub-sample delay using a Lagrange interpolating FIR; `delay` can be any non-negative real value; taps are recomputed in `settingsChanged`"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `delay` (default 0.0), `n_taps` (default 5)
- **State:** `HistoryBuffer<T> _history`, `std::vector<T> _taps`
- **Processing:** `processOne`
- **Types:** `float`, `double`

#### `AdaptiveLmsFilter<T>`
- **Description:** "Least-Mean-Squares adaptive FIR filter; updates taps online with `step_size` per sample; two inputs (signal + desired), two outputs (filtered + error)"
- **Ports:** `PortIn<T> in`, `PortIn<T> desired`, `PortOut<T> out`, `PortOut<T> error`
- **Settings:** `n_taps` (default 16), `step_size` (default 0.01)
- **State:** `HistoryBuffer<T> _xHistory`, `std::vector<T> _taps`
- **Processing:** `processBulk`
- **Types:** `float`, `double`

#### `Squelch<T>`
- **Description:** "suppresses the output when the short-term signal power falls below a threshold; outputs zeros when squelched"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `threshold` (linear power, default 0.01), `alpha` (smoothing, default 0.01)
- **State:** `T _power`
- **Processing:** `processBulk`
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

#### `Convolver<T>`
- **Description:** "fast convolution via overlap-add using an FFT; computes the linear convolution of the input with `kernel` using the gnuradio FFT algorithm"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `kernel` (FIR taps vector)
- **Processing:** `processBulk` — dynamic chunk size driven by kernel length
- **Types:** `float`, `double`

#### `SteadyStateKalman<T>`
- **Description:** "Kalman filter with steady-state gain pre-computed via DARE; constant-gain prediction-correction for time-invariant systems"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `state_dim`, `obs_dim`, `F`, `H`, `Q`, `R`, `initial_state`
- **State:** `std::vector<T> _x`
- **Processing:** `processBulk`
- **Types:** `float`, `double`

#### `KalmanFilter<T>`
- **Description:** "full linear Kalman filter with online covariance propagation; updates state estimate and error covariance each sample"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `state_dim`, `obs_dim`, `F`, `H`, `Q`, `R`, `initial_state`, `initial_covariance`
- **State:** `std::vector<T> _x`, `std::vector<T> _P`
- **Processing:** `processBulk`
- **Types:** `float`, `double`

### `RationalResampler.hpp`

#### `RationalResampler<T>`
- **Description:** "polyphase rational sample-rate converter; resamples by `interpolation / decimation` using a prototype FIR filter split into `interpolation` polyphase sub-filters; auto-designs a Kaiser-windowed sinc lowpass if `taps` is empty"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `interpolation` (default 1), `decimation` (default 1), `taps` (auto-designed if empty), `fractional_bw` (default 0.4)
- **State:** `_phases[L][nPerPhase]`, `_history[nPerPhase-1]`, `_combined[nPerPhase-1+M]`
- **Processing:** `processBulk` — `input_chunk_size = M`, `output_chunk_size = L` (GCD-reduced)
- **Types:** `float`, `double`, `std::complex<float>`, `std::complex<double>`

### `MedianFilter.hpp`

#### `MedianFilter<T>`
- **Description:** "sliding-window median filter; rejects impulse noise; returns the median of the last `window_size` samples"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `window_size` (default 5)
- **State:** `gr::HistoryBuffer<T> _history`, `std::vector<T> _scratch`
- **Processing:** `processOne` — outputs T{} until window is full, then returns median
- **Types:** `float`, `double`

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

## Module: coding (`blocks/coding/`)

### `DifferentialEncoder.hpp` / `DifferentialDecoder.hpp`

#### `DifferentialEncoder`
- **Description:** "XOR-based differential encoder: `out[n] = out[n-1] XOR in[n]`; removes phase ambiguity in BPSK/QPSK"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne`

#### `DifferentialDecoder`
- **Description:** "differential decoder: `out[n] = in[n] XOR in[n-1]`; counterpart to `DifferentialEncoder`"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne`

### `GrayCodeEncoder.hpp` / `GrayCodeDecoder.hpp`

#### `GrayCodeEncoder`
- **Description:** "encodes natural binary to Gray code: `out = n ^ (n >> 1)`"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne`

#### `GrayCodeDecoder`
- **Description:** "decodes Gray code to natural binary via iterative XOR-fold"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Processing:** `processOne`

### `PackBits.hpp` / `UnpackBits.hpp`

#### `PackBits`
- **Description:** "packs `bits_per_chunk` LSBs of each input byte into a dense output byte stream"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `bits_per_chunk` (default 1)
- **Processing:** `processBulk` — `input_chunk_size = 8 / bits_per_chunk`, `output_chunk_size = 1`

#### `UnpackBits`
- **Description:** "unpacks each input byte into `bits_per_chunk` output bytes, one bit per byte (LSB-justified)"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `bits_per_chunk` (default 1)
- **Processing:** `processBulk` — `input_chunk_size = 1`, `output_chunk_size = 8 / bits_per_chunk`

### `Scrambler.hpp`

#### `Scrambler`
- **Description:** "LFSR-based data whitener: XORs input bytes with a pseudo-random sequence; configurable polynomial `mask`, `seed`, and register `len`"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `mask` (default 0xA9), `seed` (default 0xFF), `len` (default 7)
- **Processing:** `processOne`

### `CrcCompute.hpp`

#### `CrcCompute`
- **Description:** "computes a CRC-8/16/32 checksum over the input and appends or verifies it; burst delimited by packet tags"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `poly`, `initial_value`, `mode` (append/verify)
- **Processing:** `processBulk`

### `ConvEncoder.hpp`

#### `ConvEncoder`
- **Description:** "rate-1/N convolutional encoder; one output byte per generator polynomial per input bit; shift-register length K; default polynomials are the NASA K=7 rate-1/2 code [0133, 0171]"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `constraint_length` (default 7), `generator_polynomials` (default [0133, 0171])
- **Processing:** `processBulk` — dynamic `output_chunk_size = len(polys)`

### `ViterbiDecoder.hpp`

#### `ViterbiDecoder`
- **Description:** "hard-decision Viterbi decoder for convolutional codes; block-mode ACS trellis over `traceback_depth` steps; default NASA K=7 rate-1/2 code [0133, 0171]"
- **Ports:** `PortIn<uint8_t> in`, `PortOut<uint8_t> out`
- **Settings:** `constraint_length` (default 7), `generator_polynomials` (default [0133, 0171]), `traceback_depth` (default 35)
- **State:** `_pathMetric`, `_nextMetric`, `_survivors[D][nStates]`
- **Processing:** `processBulk` — `input_chunk_size = rate * traceback_depth`, `output_chunk_size = traceback_depth`

---

## Module: demod (`blocks/demod/`)

### `PLL.hpp`

#### `PLL<T>`
- **Description:** "2nd-order phase-locked loop using the Gardner formula for α/β from normalised loop bandwidth and damping factor"
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `loop_bandwidth` (default 0.01), `damping_factor` (default 0.707)
- **State:** `T _phase`, `T _freq`, `T _alpha`, `T _beta`
- **Processing:** `processOne`
- **Types:** `float`, `double`

### `CostasLoop.hpp`

#### `CostasLoop<T>`
- **Description:** "Costas loop for carrier phase recovery; supports BPSK (order 2), QPSK (order 4), and 8PSK (order 8) phase error detectors"
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `loop_bandwidth`, `damping_factor`, `order` (default 2)
- **Processing:** `processOne`
- **Types:** `float`, `double`

### `ClockRecoveryMM.hpp`

#### `ClockRecoveryMM<T>`
- **Description:** "Mueller-Müller symbol timing recovery with variable-rate output; adjusts sampling instant using the M-M timing error detector"
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `omega` (samples/symbol), `loop_bandwidth`, `gain_mu`, `gain_omega`
- **Processing:** `processBulk` — variable output rate
- **Types:** `float`, `double`

### `SymbolSync.hpp`

#### `SymbolSync<T>`
- **Description:** "Gardner TED + PI loop filter for symbol timing synchronisation; uses `FractionalDelayLine` for sub-sample interpolation"
- **Ports:** `PortIn<std::complex<T>> in`, `PortOut<std::complex<T>> out`
- **Settings:** `sps` (samples/symbol), `loop_bandwidth`, `damping_factor`, `max_deviation`
- **Processing:** `processBulk` — variable output rate
- **Types:** `float`, `double`

---

## Module: ofdm (`blocks/ofdm/`)

### `CyclicPrefixAdd.hpp`

#### `CyclicPrefixAdd<T>`
- **Description:** "prepend a cyclic prefix to each OFDM symbol"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size`, `cp_length`
- **Processing:** `processBulk` — copies last `cp_length` samples of the input before the full symbol; dynamic `Resampling<1,1,false>` with `input_chunk_size = fft_size`, `output_chunk_size = fft_size + cp_length`

### `CyclicPrefixRemove.hpp`

#### `CyclicPrefixRemove<T>`
- **Description:** "remove the cyclic prefix from each received OFDM symbol"
- **Ports:** `PortIn<T> in`, `PortOut<T> out`
- **Settings:** `fft_size`, `cp_length`
- **Processing:** `processBulk` — discards first `cp_length` samples, copies remainder; dynamic `Resampling<1,1,false>` with `input_chunk_size = fft_size + cp_length`, `output_chunk_size = fft_size`

---

## Summary

| Module | Count |
|---|---|
| basic | 36 |
| math | 19 |
| electrical | 7 |
| fileio | 2 |
| fourier | 4 |
| filter | 20 |
| http | 2 |
| soapy | 1 |
| testing | 12 |
| ofdm | 2 |
| coding | 10 |
| demod | 4 |
| timing | 2 |
| **Total** | **121** |

**Recently added:** `Accumulator`, `AgcBlock`, `AmDemod`, `Clamp`, `DbConvert`, `EnergyDetector`, `Limiter`, `MovingAverage`, `MovingRms`, `QuadratureDemod`, `PhaseUnwrap`, `Conjugate`, `Differentiator`, `InstantaneousFrequency`, `SchmittTrigger`, `Threshold`, `Histogram` (math); `BiquadFilter`, `FractionalDelayLine`, `AdaptiveLmsFilter`, `Squelch`, `Convolver`, `SteadyStateKalman`, `KalmanFilter`, `CicDecimator`, `CicInterpolator`, `DCBlocker`, `HilbertTransform`, `Interpolator`, `Repeat`, `WienerFilter`, `MedianFilter`, `RationalResampler` (filter); `IFFT`, `SpectralEstimator`, `SpectralSubtractor` (fourier); `PhasorEstimator`, `HarmonicAnalyser`, `TotalHarmonicDistortion`, `GridFrequencyEstimator` (electrical); `DifferentialEncoder/Decoder`, `GrayCodeEncoder/Decoder`, `PackBits`, `UnpackBits`, `Scrambler`, `CrcCompute`, `ConvEncoder`, `ViterbiDecoder` (coding); `PLL`, `CostasLoop`, `ClockRecoveryMM`, `SymbolSync` (demod); `CyclicPrefixAdd`, `CyclicPrefixRemove` (ofdm); `Head`, `Skip`, `ChirpSource`, `AwgnChannel`, `StreamTagger`, `TagGate`, `TagDebugSink`, `WindowApply`, `StreamMux`, `StreamDemux`, `KeepMInN`, `StreamToVector`, `VectorToStream`, `HeaderPayloadDemux` (basic).
