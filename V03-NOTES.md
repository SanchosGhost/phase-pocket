# Phase Pocket 0.3

Influence range is now 0–150%, default 100%. Effective amplitude/bin gain is clamp(1 - Influence*(1-baseGain), 0, 1). It never becomes negative. This is attenuation depth, not output clipping. A key with magnitude 0.5 causes 50% amplitude reduction at 100%, 75% at 150%; 150% does not guarantee silence with a quiet key.

A draggable logarithmic SIDECHAIN FILTER range selects 20Hz–20kHz cutoff controls. Full Range bypasses both filters exactly. RESET or double-click the range restores Full Range. These are 12dB/oct non-resonant high-pass/low-pass filters, not brickwall selection. Cutoffs and bypass transitions are smoothed. Cutoffs are internally limited below Nyquist. The filter processes only the incoming key, before both algorithms. The oscilloscope KEY trace now displays the filtered key.

In Amplitude the selected key band controls gain of the WHOLE bass signal. It is not a multiband bass processor. In Spectrum the original phase-aware equation sees the filtered key (including filter-induced phase shift), so its budget pertains to that key, not necessarily the unfiltered kick mixed externally.

Influence is applied to each spectral bin before reconstruction and to the amplitude gain before the common delay. Parameter transitions are smoothed. Default Full Range and <=100% preserve the steady-state v0.2 equations. Common latency remains 2048 samples.

New monochrome interface: light chassis, black screens, white/grey traces, raised buttons and knobs, no key-status text. Double-click Influence resets to 100%, Smoothing to 40ms. UI sources compiled natively and three actual JUCE-rendered states inspected locally. Mac host validation remains separate.

Tests cover baseline DSP and v0.3 depth, no negative gain, detector high-pass rejection/passband, exact Full Range bypass, filter automation and four sample rates. CI runs these plus universal macOS build/signing/pluginval before packaging.

Existing absolute state values are retained. The Influence parameter range changed from 100 to 150: old normalized host automation can map differently. Check old automation or add a fresh instance. Legacy IDs remain for compatibility. This is an experimental build, not notarized and not a true-peak limiter.
