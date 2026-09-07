#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PocketLook final : public juce::LookAndFeel_V4 {
public:
    bool dark = true;
    juce::Colour ink() const;
    juce::Colour muted() const;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override;
};
class ModernDial final : public juce::Slider {
public:
    ModernDial(PocketLook&, juce::String, juce::String, juce::String, juce::Colour, bool = false);
    void paint(juce::Graphics&) override;
private:
    PocketLook& look;
    juce::String title, subtitle, unit;
    juce::Colour accent;
    bool infinity;
};
class PhasePocketAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);
    ~PhasePocketAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void setThemeForPreview(bool);
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    PhasePocketAudioProcessor& audioProcessor;
    PocketLook look;
    ModernDial influence {look,"Influence","Depth","%",juce::Colour(0xff5987ff)};
    ModernDial smoothing {look,"Smoothing","Movement","ms",juce::Colour(0xff30d8ce)};
    ModernDial duration {look,"Duration","Sidechain length","ms",juce::Colour(0xff8971ff),true};
    ModernDial sustain {look,"Sustain","Tail amount","%",juce::Colour(0xff35bdef)};
    juce::Slider sidechainRange, midSide;
    juce::TextButton amplitude {"Amplitude"}, spectrum {"Spectrum"};
    juce::TextButton themeButton {"theme"}, bypassButton {"power"}, resetFilter {"Reset"};
    std::unique_ptr<SliderAttachment> influenceAttach, smoothingAttach, durationAttach, sustainAttach, msAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;
    std::unique_ptr<juce::ParameterAttachment> lowAttach, highAttach;
    std::unique_ptr<juce::PropertiesFile> preferences;
    std::array<PocketTrace,4096> history {};
    SpectrumTrace spectrumCurve;
    int cursor = 0, filled = 0;
    bool rangeGesture = false;
    void timerCallback() override;
    void setMode(float);
    void syncRange();
    void drawPanel(juce::Graphics&, juce::Rectangle<float>);
    void drawGraph(juce::Graphics&, bool);
    void drawScope(juce::Graphics&);
    juce::Rectangle<int> scaled(float,float,float,float) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
