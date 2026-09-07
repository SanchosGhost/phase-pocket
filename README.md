# Phase Pocket 0.6

Experimental JUCE 8.0.4 sidechain VST3 by Rainline Music.

## v0.6

- Dynamic VST3 latency is driven by the Mode parameter itself: Amplitude reports 0 samples and Spectrum reports 2048 samples. UI clicks, host automation and restored state all use the same message-thread notification path.
- Spectrum adds a delayed 12 ms amplitude transient guard before handing control to the FFT reduction curve. This reduces the initial kick/bass sum peak but is not a true-peak limiter.
- Smoothing controls movement inside an active event and no longer extends the sidechain after the event has ended. Event end uses a short 4 ms fade and an adaptive, peak-relative detector.
- New Duration and Sustain parameters are appended after all existing IDs. Duration defaults to infinity (the 2000 ms maximum position); Sustain defaults to 100%, preserving existing sessions. Finite Duration shortens drum sidechains and Sustain controls the retained tail.
- The editor has a new dark navy visual system based on layered gradients, recessed panels, electric-blue/cyan/violet accents, four large controls, spectral response and one-second output/key scope.
- CI runs baseline/v0.4/v0.5/v0.6 DSP regression, universal macOS and Windows x64 builds, and pluginval strictness 5 on pushes and pull requests.

## Processing

Amplitude follows the current filtered sidechain envelope with zero added algorithmic latency. Spectrum uses a 2048-sample FFT reconstruction with 512-sample hops and reports 2048 samples of latency. Mode changes notify the VST3 host from the message thread; the host may briefly interrupt playback while rebuilding delay compensation.

Influence 0–100 is linear depth; 100–150 is exponential: 100=1x, 125=2.828x, 150=8x. Gain never becomes negative. Spectrum uses 32 overlapping log-energy bands.

The key filter is a non-resonant 12dB/oct HP+LP. Full-range endpoints bypass it. M/S focus changes processing depth, not output level. Mono input has no Side component.

## Builds

GitHub Actions builds macOS universal arm64+x86_64 and Windows x64 VST3 packages. macOS builds are ad-hoc signed, not notarized; Windows builds are unsigned.

This is experimental software. Back up old projects and plug-ins before replacement.
