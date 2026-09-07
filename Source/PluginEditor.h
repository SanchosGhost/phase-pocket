#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class PocketLook final : public juce::LookAndFeel_V4 {
public:
    PocketLook();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    juce::Font getTextButtonFont(juce::TextButton&,int) override;
};
class KeyBand final : public juce::Component {
public:
    explicit KeyBand(juce::AudioProcessorValueTreeState&);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void sync();
private:
    juce::AudioProcessorValueTreeState& state;
    float low=20,high=20000;int active=-1;
    float position(float) const;
    void apply(float);
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
    KeyBand keyBand;
    juce::Slider influence,smoothing;
    juce::TextButton spectrum{"SPECTRUM"},amplitude{"AMPLITUDE"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttach,smoothAttach;
    std::array<PocketTrace,600> history{};
    int cursor=0;bool amplitudeMode=false;
    juce::TooltipWindow tips{this,700};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
