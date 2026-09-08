#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
enum class PocketTheme { Neon, SolidDark, SolidWhite };
class PocketLook final:public juce::LookAndFeel_V4 {
public:
    PocketTheme theme=PocketTheme::Neon;
    bool isDark() const{return theme!=PocketTheme::SolidWhite;}bool isNeon() const{return theme==PocketTheme::Neon;}
    juce::Colour pick(juce::uint32 neon,juce::uint32 dark,juce::uint32 white) const;
    juce::Colour ink() const;juce::Colour muted() const;
    juce::Font getTextButtonFont(juce::TextButton&,int) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&) override;
};
class ModernDial final:public juce::Slider {
public:
    ModernDial(PocketLook&,juce::String,juce::String,juce::String,juce::uint32,bool=false);void paint(juce::Graphics&) override;
private:
    PocketLook& look;juce::String title,subtitle,unit;juce::uint32 accent;bool infinity;
};
class PhasePocketAudioProcessorEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);~PhasePocketAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;void paintOverChildren(juce::Graphics&) override;void resized() override;
    void setPanelExpanded(bool);
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    PhasePocketAudioProcessor& audioProcessor;PocketLook look;
    ModernDial influence{look,"Influence","Depth","%",0xff5987ff};
    ModernDial duration{look,"Duration","Sidechain length","ms",0xff32d4cb,true};
    juce::Slider sidechainRange,processingRange,midSide;
    juce::TextButton settingsButton{"settings"},bypassButton{"power"},panelButton{"panel"},resetFilter{"Reset"},resetProcessing{"Reset"};
    std::unique_ptr<SliderAttachment> influenceAttach,durationAttach,msAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
    std::unique_ptr<juce::ParameterAttachment> lowAttach,highAttach,processLowAttach,processHighAttach;
    std::unique_ptr<juce::PropertiesFile> preferences;
    std::array<PocketTrace,8192> history{};int cursor=0,filled=0;
    bool expanded=false,ready=false,rangeGesture=false,processRangeGesture=false,capturingBlur=false,bypassTarget=false;
    double resizeStamp=0,lastFrameMs=0;float bypassMix=0;double gainWindow=.5,scopeWindow=1.;
    juce::Image blurredSnapshot;juce::Rectangle<int> blurArea;
    void timerCallback() override;void syncRange();void syncProcessingRange();void saveSize();
    void setTheme(PocketTheme,bool persist=true);void showSettingsMenu();void setHistoryWindow(bool gain,double seconds);void captureBlurSnapshot();
    void panel(juce::Graphics&,juce::Rectangle<float>);void graph(juce::Graphics&,juce::Rectangle<float>,bool);
    juce::Rectangle<int> scaled(float,float,float,float) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
