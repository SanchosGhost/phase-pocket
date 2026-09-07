# Phase Pocket 0.4

Experimental JUCE 8.0.4 sidechain VST3 by Rainline Music. Default: Amplitude, Influence 100%, Smoothing 40ms, Full Range sidechain. GUI order Amplitude / Spectrum; saved mode indices remain Spectrum=0, Amplitude=1 for automation compatibility.

## Sound
Influence 0–100% keeps the amplitude behavior from v0.3. From 100 to 150 it is exponentially boosted: 100%=1x, 125%=2.828x, 150%=8x. Amplitude gain is max(0,1-depth*envelope). No negative gain or polarity inversion. A quiet key is not auto-normalized; exact silence is not guaranteed at any key level. Smoothing is envelope recovery, not an output ceiling.

Spectrum now uses 32 overlapping log-frequency energy bands, neighborhood averaging and a smooth dB curve, rather than the old phase-dependent per-bin quadratic budget. Base attenuation = 24*keyEnergy/(bassEnergy+keyEnergy+softFloor) dB, multiplied by effective depth and capped at 60dB. Smoothing controls band recovery. This follows the broad idea of sidechain-driven spectral EQ, not Trackspacer's proprietary implementation. It is not a summing/true-peak limiter and does not promise an unchanged perceived kick timbre.

The key-only 12dB/oct HP/LP filter is unchanged; 20Hz/20kHz extremes bypass it. It does not directly filter the bass. Both modes remain stereo-linked with 2048 samples reported latency. Bypass is a host-integrated parameter appended after existing IDs, with a smoothed delayed-dry path.

## UI
Light/Dark use shared geometry. Moon/sun button precedes bypass. Theme persists in local phasePocket.ui.theme preferences, not an audio preset. Multiple open editors in a process share message-thread changes. No file access in audio callback.

One-second scope: muted blue input bass, bright white filtered key, no In/Key/Out text. Gain history remains 500ms and is an applied-gain estimate in Spectrum, not LUFS. Telemetry uses timed, signed min/max samples through a bounded SPSC FIFO. Drawing clips visual over-range without altering audio.

Vertical rotary drag; Shift fine drag; double-click number to enter (comma/point accepted), double-click outside number resets default. Arrow/Home/End and focus-only mouse wheel supported. Filter handles are keyboard focusable, readouts accept Hz/kHz by double-click, Reset affects only filter. UI scales 900x672 to 1800x1344 with fixed aspect ratio; default 1200x896.

## Build
GitHub Actions builds macOS universal (arm64 + x86_64, macOS 11+) and Windows x64 VST3, runs DSP regression and pluginval strictness 5, then packages only successful validations. Artifacts: PhasePocket-v0.4-macOS-Universal-VST3 and PhasePocket-v0.4-Windows-x64-VST3. Ad-hoc macOS signing is not Apple notarization. Windows binary is unsigned.

Local CMake: cmake -S . -B build -DJUCE_DIR=/path/to/JUCE, then cmake --build build --config Release --target PhasePocket_VST3. Windows uses Visual Studio 2022 x64, macOS may add -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64".

## Validation and limits
Local DSP tests passed at 44.1/48/96/192kHz, including boosted depth, exact Full Range filter bypass, key rejection/passband, phase-polarity invariance, spectral band selectivity, latency and delayed bypass. Native JUCE processor/editor compiled on Linux and actual screenshots generated at all requested sizes and 2x. This is not Windows/macOS host QA. CI results must be checked in Actions; they were not available from the connected source during implementation.

Inter could not be downloaded in the offline local environment: temporary system Arial on macOS/Windows and Liberation Sans for Linux previews, explicitly permitted by the spec's fallback. No unlicensed font embedding. No new third-party dependency. Full screen-reader semantics for custom frequency handles and target-host interaction testing remain to be completed. Address/undefined sanitizer executable could not run locally because libasan.so.6 is missing.

Existing parameter IDs, order, ranges and stored values remain; mode default changes only for fresh instances. Existing Spectrum and >100% presets intentionally sound different due to the requested new algorithms. Duplicate the old plugin/project before replacing if exact recall matters. VST3 identity is unchanged.

## Design references
Applied the supplied native UI specification, not a generated image background. Removed decorative gradients, fake hardware/shadows and redundant cards. Practical critiques, not a reliable AI-authorship detector:
- https://alexlavaee.me/blog/lessons-learned-designing-with-ai/
- https://smoothui.dev/blog/ai-design-slop
- https://www.wavesfactory.com/audio-plugins/trackspacer/ (public description of 32-band sidechain EQ)

Contrast adjustment permitted by specification: Light accent #5598C7 -> #4D8FBE (2.92 -> 3.27 against window); thumb border #8D9BA8 -> #7E8C99 (2.84 -> 3.44 against white). Secondary text contrast 5.71 Light and 8.14 Dark. Other tokens unchanged. Scope/mode changes follow the latest chat rather than the older specification.
