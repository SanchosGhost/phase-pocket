# Phase Pocket 0.2

Two-mode sidechain VST3 prototype. Insert on bass/main input; route kick into the external sidechain. Outputs processed bass only, not the kick.

## Controls

- Spectrum / Amplitude buttons.
- Influence 0–100%, default 100%: blends delayed dry with processed signal.
- Smoothing 0–500ms, default 40ms: release time constant, not total fade duration. Larger values extend recovery; attack remains fast.

## Spectrum

For each complex bin B (bass) and K (kick), T=max(abs(B),abs(K)). Solve abs(K+g*B)<=T for largest g in [0,1], including Re(B*conj(K)). No tolerance or attenuation floor. Stereo-linked gain uses the stricter channel. Fixed 20Hz–20kHz range, limited by Nyquist. 2048-point STFT with sqrt-Hann and 75% overlap. Zero-padded starting frames fix the v0.1 startup fade. Smoothing controls per-bin gain recovery.

100% means full application of this formula, NOT erasing every frequency occupied by the kick. Destructive interference may require no ducking. This is neither complex subtraction B-K nor magnitude subtraction. Spectral budgets do NOT guarantee time-domain sample peaks or true peaks below 0 dBFS.

## Amplitude

Detector = clamp(max(abs(K_L),abs(K_R)),0,1). With no smoothing, wet=B*(1-detector). With smoothing, e[n]=max(detector[n], e[n-1]*exp(-1/(sampleRate*tau))). Output = dry + Influence*(wet-dry).

At 100% Influence, a 0dBFS key sample fully ducks the main input; a -6dBFS key peak gives approximately half gain. No auto-normalization. Zero smoothing is audio-rate amplitude modulation and may sound distorted; this behavior is inherent, not a limiter. Smoothing reduces rapid recovery fluctuations but can still change the timbre.

## Timing and display

Both modes run continuously with 2048 samples of reported latency (42.67ms at 48kHz). Mode and Influence automation use a 5ms one-pole transition. Correct host PDC and routing are required for the external kick to align with bass. No processing limiter.

Top display: 500ms gain history; Spectrum is an energy-weighted spectral gain ESTIMATE, not a loudness measurement. Bottom display: aligned L/mono input, key, output min/max waveform columns, fixed +/-1 scale. Visual clipping does not clip the audio. All traces come from actual audio data. A bounded SPSC queue drops visualization frames rather than blocking audio if the editor is closed/slow.

## Old projects

VST3 identity retained. Existing stored Influence and Release values are preserved; insert a fresh instance for 100%/40ms defaults. The old release ID now drives Smoothing with an extended range, so old normalized automation may need revision. Legacy Tolerance/Low/High/MaxReduction/PhaseAware IDs are retained in the host parameter list for compatibility but do not affect DSP. Old sessions without Mode default to Spectrum.

## Validation and build

Local core tests passed at 44.1, 48, 96 and 192kHz: unity startup/reconstruction, exact dry at 0%, latency, amplitude equation, stereo linking, smoothing, spectral root and finite mode automation. These tests use the same PocketDSP.h as the plugin. Native JUCE processor/editor sources also compiled in Linux. This is not proof of macOS host compatibility or subjective sound quality.

GitHub Actions builds universal arm64+x86_64 macOS VST3, ad-hoc signs it, and runs pluginval 1.0.4 strictness 5. The plugin artifact is only packaged after those steps succeed. Check Actions results; do not assume a pushed commit is a passed build. There is no Apple Developer ID notarization. GitHub wraps the plugin ZIP in an artifact ZIP; extract both layers.

Local core tests:

```bash
c++ -std=c++17 -O2 Tests/dsp_test.cpp -o dsp_test
./dsp_test
```

Local Mac build: Xcode, CMake 3.22+, JUCE 8.0.4 beside this project, then `bash build_macos.sh`. Review JUCE and bundled SDK licensing and applicable patent rights before commercial distribution.
