#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
JUCE_DIR="${JUCE_DIR:-$ROOT/../JUCE}"
cmake -S "$ROOT" -B "$ROOT/build-macos" -G Xcode -DJUCE_DIR="$JUCE_DIR"
cmake --build "$ROOT/build-macos" --config Release --target PhasePocket_VST3
printf '\nBuilt bundle:\n%s\n' "$ROOT/build-macos/PhasePocket_artefacts/Release/VST3/Phase Pocket.vst3"
