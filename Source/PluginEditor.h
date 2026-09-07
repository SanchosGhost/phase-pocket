#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
struct ThemeTokens {
 enum Key {windowBg,analyzerBg,controlSurface,textPrimary,textSecondary,analyzerText,analyzerSecondary,accent,inactiveTrack,separator,analyzerGrid,graphMain,waveIn,waveKey,waveOut,thumbFill,thumbBorder,segmentBg,segmentSelected,hoverSurface,iconButtonSurface,iconButtonHover,iconInk,focusRing};
 std::array<juce::Colour,24> c;bool dark=false;explicit ThemeTokens(bool=false);juce::Colour operator[](Key k) const {return c[(size_t)k];}
};
class ThemeStore : public juce::ChangeBroadcaster {public:ThemeStore();void set(bool);bool dark=false;private:std::unique_ptr<juce::PropertiesFile> file;};
class Dial : public juce::Slider {
public:
 Dial(juce::AudioProcessorValueTreeState&,const juce::String&,const juce::String&,const juce::String&,const juce::String&,ThemeTokens&);~Dial() override;
 void paint(juce::Graphics&) override;void mouseDown(const juce::MouseEvent&) override;void mouseDrag(const juce::MouseEvent&) override;void mouseUp(const juce::MouseEvent&) override;void mouseDoubleClick(const juce::MouseEvent&) override;void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&) override;bool keyPressed(const juce::KeyPress&) override;void focusLost(FocusChangeType) override;
private:
 void end();void complete(float);void edit();void commit();ThemeTokens& theme;juce::RangedAudioParameter& param;juce::ParameterAttachment attachment;juce::String title,subtitle,unit;juce::TextEditor entry;float startP=0;bool gesture=false;
};
class RangeControl : public juce::Component,public juce::SettableTooltipClient {
public:
 RangeControl(juce::AudioProcessorValueTreeState&,ThemeTokens&);~RangeControl() override;void paint(juce::Graphics&) override;void resized() override;void mouseDown(const juce::MouseEvent&) override;void mouseDrag(const juce::MouseEvent&) override;void mouseUp(const juce::MouseEvent&) override;void mouseDoubleClick(const juce::MouseEvent&) override;
private:
 class Handle : public juce::Component {public:Handle(RangeControl& r,int i):owner(r),index(i){setWantsKeyboardFocus(true);setName(i?"High cut":"Low cut");}bool keyPressed(const juce::KeyPress&) override;void paint(juce::Graphics&) override;void mouseDown(const juce::MouseEvent&) override;void mouseDrag(const juce::MouseEvent&) override;void mouseUp(const juce::MouseEvent&) override;RangeControl& owner;int index;};
 void finish();void set(int,float,bool=false);void drag(float);void reset();void edit(int);void commit();float x(float) const;
 ThemeTokens& theme;juce::RangedAudioParameter &lowParam,&highParam;juce::ParameterAttachment lowAttach,highAttach;float low=20,high=20000;int active=-1,lastActive=0,editing=0;bool gesture=false;Handle lowHandle{*this,0},highHandle{*this,1};juce::TextButton resetButton{"Reset"};juce::TextEditor entry;
};
class PhasePocketAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer,private juce::ChangeListener {
public:
 explicit PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor&);~PhasePocketAudioProcessorEditor() override;void paint(juce::Graphics&) override;void resized() override;void refreshTrace();void setThemeForPreview(bool);
private:
 class FlatLook : public juce::LookAndFeel_V4 {public:explicit FlatLook(ThemeTokens& t):theme(t){}void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;juce::Font getTextButtonFont(juce::TextButton&,int) override;ThemeTokens& theme;};
 class IconButton : public juce::Button {public:IconButton(ThemeTokens& t,bool power):Button(power?"Bypass":"Theme"),theme(t),isPower(power){}void paintButton(juce::Graphics&,bool,bool) override;ThemeTokens& theme;bool isPower;};
 void timerCallback() override {if(isShowing())refreshTrace();}void changeListenerCallback(juce::ChangeBroadcaster*) override;void applyTheme();
 PhasePocketAudioProcessor& processor;juce::SharedResourcePointer<ThemeStore> themes;ThemeTokens theme;FlatLook look{theme};juce::Component canvas;Dial influence,smoothing;RangeControl range;
 class ModeButton : public juce::TextButton {public:using TextButton::TextButton;ModeButton* peer=nullptr;bool keyPressed(const juce::KeyPress& k) override {if(peer&&(k.getKeyCode()==juce::KeyPress::leftKey||k.getKeyCode()==juce::KeyPress::rightKey)){peer->triggerClick();peer->grabKeyboardFocus();return true;}return TextButton::keyPressed(k);}};
 ModeButton amplitude{"Amplitude"},spectrum{"Spectrum"};IconButton themeButton{theme,false},bypassButton{theme,true};juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttach;juce::ParameterAttachment modeAttach;std::array<PocketTrace,4096> history{};int cursor=0,filled=0;juce::TooltipWindow tips{this,500};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhasePocketAudioProcessorEditor)
};
