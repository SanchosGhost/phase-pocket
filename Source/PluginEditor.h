#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ModernDial final : public juce::Slider
{
public:
    ModernDial(juce::String title, juce::String subtitle, juce::String unit,
               juce::Colour accent, bool infinityAtMaximum = false);
    void paint(juce::Graphics&) override;
private:
    juce::String title, subtitle, unit;
    juce::Colour accent;
    bool infinityAtMaximum = false;
};

class RangeLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider&) override;
};

class PhasePocketAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);
    ~PhasePocketAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    void timerCallback() override;
    void setMode(float);
    void setSidechainRangeParameter(const char*, float);
    void drawPanel(juce::Graphics&, juce::Rectangle<float>, float = 18.0f);
    void drawSpectrum(juce::Graphics&, juce::Rectangle<float>);
    void drawScope(juce::Graphics&, juce::Rectangle<float>);
    juce::Rectangle<int> scaled(float, float, float, float) const;
    PhasePocketAudioProcessor& audioProcessor;
    ModernDial influence { "Influence", "Depth", "%", juce::Colour(0xff5b83ff) };
    ModernDial smoothing { "Smoothing", "Movement", "ms", juce::Colour(0xff31ddd2) };
    ModernDial duration { "Duration", "Sidechain length", "ms", juce::Colour(0xff7c72ff), true };
    ModernDial sustain { "Sustain", "Tail amount", "%", juce::Colour(0xff38bdf8) };
    RangeLookAndFeel rangeLookAndFeel;
    juce::Slider sidechainRange, midSide;
    juce::TextButton amplitude { "Amplitude" }, spectrum { "Spectrum" };
    juce::TextButton themeButton { "Moon" }, bypassButton { "Power" };
    std::unique_ptr<SliderAttachment> influenceAttach, smoothingAttach;
    std::unique_ptr<SliderAttachment> durationAttach, sustainAttach, msAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach;
    SpectrumTrace spectrumCurve;
    std::array<PocketTrace, 4096> history {};
    int cursor = 0, filled = 0;
    bool bypassLook = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
