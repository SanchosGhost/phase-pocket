# Phase Pocket 0.5

Experimental JUCE 8.0.4 sidechain VST3 by Rainline Music. Default Amplitude, 100% Influence, 40ms Smoothing, Full Range filter, centred M/S.

## v0.5
- Scope now shows actual processed L/mono output in muted blue and filtered key in white, one second.
- Bypass desaturates the entire interface and adds a translucent grey overlay without blocking the bypass button.
- Spectrum replaces history with current spectral reduction curves for Mid and Side, 20Hz–20kHz log frequency, 0dB top and negative infinity bottom. The ordinate compresses dB toward infinity (not evenly spaced dB); labels are explicitly positioned. Curves come from real band gains, not a simulated spectrum. They describe the current control curve, not a static measurement of the time-varying overlap-add system.
- M/S replaces Stereo linked. Left processes only Mid and leaves Side dry; centre processes both; right processes only Side and leaves Mid dry. Intermediate positions reduce the other component's processing depth, not its output level. The added msBalance parameter is appended after existing IDs and has default zero. M/S changes are smoothed.
- First-ever editor size is 900x672. Resize persists per instance and in project state, plus a local fallback for new instances. Theme/size file writes stay outside audio callback.

## Latency: honest split, not cosmetic relabelling
Amplitude now uses the current input and current detector envelope, with ZERO samples of added algorithmic latency. Its steady-state gain equation is unchanged; the unnecessary 2048-sample delay was removed. The detector sidechain HP/LP phase response and smoothing are not audio buffering latency.

Spectrum deliberately keeps the v0.4 FFT reconstruction and 2048-sample latency (42.67ms at 48kHz). Cutting the FFT window to 1–2ms loses low-frequency resolution. A causal IIR architecture could remove buffering but would change phase/time response; that was not silently substituted under a promise of no quality loss. The latency label is removed, but the real values are reported to the host.

IMPORTANT: mode changes also change latency. Host notification occurs outside audio callback, from the message thread (immediate on UI click, polled at 30Hz for host parameter changes), then the active engine is switched. Change modes with transport stopped and allow host PDC to settle. Seamless/sample-accurate mode automation and mode changes during fast offline bounce are NOT supported/validated. Set the mode before rendering. No crossfade between differently delayed signals is used, avoiding a deliberate comb-filter transition.

## Core behavior
Influence 0–100 is linear depth; 100–150 is exponential: 100=1x, 125=2.828x, 150=8x. Gain never becomes negative. Spectrum uses 32 overlapping log-energy bands and recovery smoothing, as introduced in v0.4; there is no phase-dependent quadratic summation budget in the active path. This is not a true-peak limiter and cannot guarantee an unchanged perceived kick timbre.

Key filter: non-resonant 12dB/oct HP+LP, full-range endpoints bypass. It filters only the key. Unselected M/S components remain unprocessed but retain Spectrum's common alignment delay. Mono input has no Side component.

## Builds and tests
Actions builds macOS universal arm64+x86_64 and Windows x64 VST3. Both run baseline/v0.4/v0.5 DSP regression and pluginval strictness 5 before packaging. Artifacts: PhasePocket-v0.5-macOS-Universal-VST3 and PhasePocket-v0.5-Windows-x64-VST3. Current CI completion is not asserted by the source update. macOS ad-hoc signing is not notarization; Windows is unsigned.

Local tests passed: actual zero-delay impulse/amplitude response; 44.1/48/96/192kHz; stereo M/S isolation and mixed M/S signal; flat response for unprocessed component; baseline filter/spectral tests. Native JUCE editor compiled and resize/reopen/project size persistence and reported per-mode latency tested in a Linux harness. Target DAW PDC behavior remains unverified.

Temporary system Arial on macOS/Windows and Liberation Sans on Linux remain; no new font/dependency. Inter embedding and complete accessibility/target-host interaction audit remain unfinished. Existing parameter IDs/order/ranges are preserved; old projects intentionally acquire lower Amplitude latency. Back up old projects/plugins before replacement.
