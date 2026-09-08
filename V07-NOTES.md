# v0.7 — amplitude-only

Supersedes spectral/dynamic-mode descriptions in the v0.3–v0.6 notes and README.

## Processing
- FFT and spectral processing have been removed. One amplitude engine, no mode switch.
- Lookahead = ceil(sample rate × 0.001) samples: 48 at 48 kHz, 45 at 44.1 kHz. The processor reports this latency to the host. Dry bypass and plotted signals share that delay.
- A sliding future-peak window catches the first attack. This is a ducker, not a true-peak limiter or a guarantee against clipping in the final mix.
- Duration defaults to infinity (2000 ms parameter endpoint). Finite values cover 1–1999 ms.
- Duration is total event length. The last 20% is a half-cosine fade to zero: 200 ms = 160 ms hold + 40 ms fade; 100 ms = 80 + 20; 1 ms = 0.8 + 0.2, quantized to samples.
- Smoothing is fixed at 40 ms internally. The finite-duration gate is applied AFTER smoothing, so release cannot extend the requested duration.
- Sustain is fixed at zero. No Sustain or Smoothing knobs.
- Old parameter slots are retained for session/automation mapping but mode, sustain and release no longer affect DSP. Existing projects will sound different where they used removed features. No FFT implementation remains in the live source.

## Interface
- Two stacked dials: Influence and Duration.
- Equal-sized gain-history and oscilloscope panels. Gain percentages on the right; white on charcoal in light mode.
- Sidechain filter and M/S start collapsed on every editor opening. Bottom-left double chevron expands/collapses the panel and flips direction.
- Bypass desaturates the palette and displays BYPASSED. Controls remain usable, including re-enable.
- Width is stored per processor/project and in user preferences. Constructor resizes cannot overwrite the restored width. Height follows the current collapsed/expanded aspect ratio.
- Windows requests Segoe UI Variable Text / Semilight; falls back to Segoe UI if the variable family is absent. macOS requests SF Pro Text / SF Pro, with the system sans-serif fallback. Fonts are not bundled.

## Validation
`c++ -std=c++17 -O2 Tests/v07_test.cpp -o v07_test && ./v07_test`

Regression covers delayed dry identity, first-sample attack, exact finite lengths including 1 ms, proportional fade, sustained-key timeout, infinity, natural key end, M/S exclusion and bypass at 44.1/48/96/192 kHz. Old spectral/zero-latency tests are historical and are superseded by this suite.

CI builds macOS universal and Windows x64 VST3, then runs pluginval strictness 5. Full JUCE compilation, host compensation and UI appearance still require those builds and DAW checks.
