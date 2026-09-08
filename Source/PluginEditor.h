#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class PocketLook final:public juce::LookAndFeel_V4 {
public:
    bool dark=true,disabled=false;
    juce::Colour colour(juce::uint32 darkColour,juce::uint32 lightColour) const;
    juce::Colour ink() const;juce::Colour muted() const;
    juce::Font getTextButtonFont(juce::TextButton&,int) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&) override;
};
class ModernDial final:public juce::Slider {
public:
    ModernDial(PocketLook&,juce::String,juce::String,juce::String,juce::uint32,bool=false);
    void paint(juce::Graphics&) override;
private:
    PocketLook& look;juce::String title,subtitle,unit;juce::uint32 accent;bool infinity;
};
class PhasePocketAudioProcessorEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);
    ~PhasePocketAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;void resized() override;
    void setThemeForPreview(bool);void setPanelExpanded(bool);
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    PhasePocketAudioProcessor& audioProcessor;PocketLook look;
    ModernDial influence{look,"Influence","Depth","%",0xff5987ff};
    ModernDial duration{look,"Duration","Sidechain length","ms",0xff32d4cb,true};
    juce::Slider sidechainRange,midSide;
    juce::TextButton themeButton{"theme"},bypassButton{"power"},panelButton{"panel"},resetFilter{"Reset"};
    std::unique_ptr<SliderAttachment> influenceAttach,durationAttach,msAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
    std::unique_ptr<juce::ParameterAttachment> lowAttach,highAttach;
    std::unique_ptr<juce::PropertiesFile> preferences;
    std::array<PocketTrace,4096> history{};int cursor=0,filled=0;
    bool expanded=false,ready=false,rangeGesture=false;double resizeStamp=0;
    void timerCallback() override;void syncRange();void saveSize();
    void panel(juce::Graphics&,juce::Rectangle<float>);void graph(juce::Graphics&,juce::Rectangle<float>,bool);
    juce::Rectangle<int> scaled(float,float,float,float) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
