#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class PocketLook final : public juce::LookAndFeel_V4 {
public:
    PocketLook();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    juce::Font getTextButtonFont(juce::TextButton&,int) override;
};
class PhasePocketAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);
    ~PhasePocketAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void refreshTrace();
private:
    void timerCallback() override {refreshTrace();}
    PhasePocketAudioProcessor& processor;
    PocketLook look;
    juce::Slider influence,smoothing;
    juce::TextButton spectrum{"SPECTRUM"},amplitude{"AMPLITUDE"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttach,smoothAttach;
    std::array<PocketTrace,600> history{};
    int cursor=0;
    float keyHold=0;
    bool amplitudeMode=false;
    juce::TooltipWindow tips{this,700};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
