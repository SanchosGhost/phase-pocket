cmake_minimum_required(VERSION 3.22)
project(PhasePocket VERSION 0.3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../JUCE")
endif()
add_subdirectory(${JUCE_DIR} JUCE)
juce_add_plugin(PhasePocket
    COMPANY_NAME "Rainline Music"
    BUNDLE_ID "ru.rainlinemusic.phasepocket"
    PLUGIN_MANUFACTURER_CODE RnLn
    PLUGIN_CODE PhPk
    FORMATS VST3 Standalone
    PRODUCT_NAME "Phase Pocket"
    DESCRIPTION "Dual-mode spectral and amplitude sidechain"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics)
juce_generate_juce_header(PhasePocket)
target_sources(PhasePocket PRIVATE Source/SidechainFilter.h Source/PocketDSP.h Source/PluginProcessor.cpp Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginEditor.h)
target_compile_definitions(PhasePocket PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(PhasePocket PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
