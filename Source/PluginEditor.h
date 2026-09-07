#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PhasePocketLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    PhasePocketLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
};

class PhasePocketAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);
    ~PhasePocketAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void configureKnob(juce::Slider&, juce::Label&, const juce::String&);
    PhasePocketAudioProcessor& processor;
    PhasePocketLookAndFeel lookAndFeel;
    juce::Slider amount, tolerance, low, high, maxReduction, release;
    juce::Label amountLabel, toleranceLabel, lowLabel, highLabel, maxReductionLabel, releaseLabel;
    juce::ToggleButton phaseAware { "PHASE AWARE" };
    juce::Label reductionReadout;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> amountAttachment, toleranceAttachment, lowAttachment, highAttachment, maxReductionAttachment, releaseAttachment;
    std::unique_ptr<ButtonAttachment> phaseAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
