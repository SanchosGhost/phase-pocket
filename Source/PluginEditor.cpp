#include "PluginEditor.h"
namespace ui {
const juce::Colour bg{0xff151c20},panel{0xff1c252b},screen{0xff11191e},border{0xff33434c},text{0xffe5eef2},muted{0xffa1b3bd},teal{0xff69dfd0},orange{0xffffb779};
juce::Font font(float size,bool bold=false){
#if JUCE_MAC
    return juce::Font(juce::FontOptions("Avenir Next",size,bold?juce::Font::bold:juce::Font::plain));
#else
    return juce::Font(juce::FontOptions("Liberation Sans",size,bold?juce::Font::bold:juce::Font::plain));
#endif
}
void label(juce::Graphics& g,juce::String t,juce::Rectangle<int> r,float size=14,juce::Colour colour=muted,int align=juce::Justification::centredLeft){g.setColour(colour);g.setFont(font(size));g.drawText(t,r,align);}
void box(juce::Graphics& g,juce::Rectangle<float> r,juce::Colour colour){g.setColour(colour);g.fillRoundedRectangle(r,10);g.setColour(border);g.drawRoundedRectangle(r,10,1);}
}
PocketLook::PocketLook(){
    setDefaultSansSerifTypefaceName(juce::Font::getDefaultSansSerifFontName());
    setColour(juce::Slider::textBoxTextColourId,ui::text);setColour(juce::Slider::textBoxBackgroundColourId,ui::screen);
    setColour(juce::Slider::textBoxOutlineColourId,ui::border);setColour(juce::Slider::textBoxHighlightColourId,ui::teal.withAlpha(0.3f));
    setColour(juce::TextButton::buttonColourId,ui::panel);setColour(juce::TextButton::buttonOnColourId,ui::teal);
    setColour(juce::TextButton::textColourOffId,ui::muted);setColour(juce::TextButton::textColourOnId,ui::bg);
}
juce::Font PocketLook::getTextButtonFont(juce::TextButton&,int){return ui::font(14,true);}
void PocketLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider&){
    const float side=(float)juce::jmin(w,h)-18;
    auto r=juce::Rectangle<float>(side,side).withCentre({x+w*0.5f,y+h*0.5f});
    auto centre=r.getCentre();const float radius=side*0.5f;
    juce::Path track,arc;track.addCentredArc(centre.x,centre.y,radius,radius,0,start,end,true);
    g.setColour(ui::border);g.strokePath(track,juce::PathStrokeType(3));
    arc.addCentredArc(centre.x,centre.y,radius,radius,0,start,start+value*(end-start),true);
    g.setColour(ui::teal);g.strokePath(arc,juce::PathStrokeType(3,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    auto face=r.reduced(9);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3b4952),face.getTopLeft(),juce::Colour(0xff202b32),face.getBottomRight(),false));g.fillEllipse(face);
    g.setColour(juce::Colour(0xff52636b));g.drawEllipse(face,1);
    const float a=start+value*(end-start);
    const auto p=centre+juce::Point<float>(std::sin(a),-std::cos(a))*(radius-17);
    const auto q=centre+juce::Point<float>(std::sin(a),-std::cos(a))*(radius-29);
    g.setColour(ui::text);g.drawLine({p,q},3);
}
PhasePocketAudioProcessorEditor::PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
    setLookAndFeel(&look);setSize(920,620);
    for(auto* s:{&influence,&smoothing}){s->setSliderStyle(juce::Slider::RotaryVerticalDrag);s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,140,30);s->setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);addAndMakeVisible(*s);}
    influence.setTextValueSuffix(" %");smoothing.setTextValueSuffix(" ms");influence.setName("Influence");smoothing.setName("Smoothing");
    influence.setTooltip("0%: delayed original. 100%: full action of the selected algorithm.");
    smoothing.setTooltip("Release time constant. 0 ms follows the key most closely; larger values extend the carve.");
    amountAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,"amount",influence);
    smoothAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,"release",smoothing);
    for(auto* b:{&spectrum,&amplitude})addAndMakeVisible(*b);
    auto choose=[this](int index){auto* m=processor.parameters.getParameter("mode");m->beginChangeGesture();m->setValueNotifyingHost(m->convertTo0to1((float)index));m->endChangeGesture();refreshTrace();};
    spectrum.onClick=[choose]{choose(0);};amplitude.onClick=[choose]{choose(1);};
    spectrum.setTooltip("Phase-aware spectral budget, 20 Hz to 20 kHz (up to Nyquist).");
    amplitude.setTooltip("Full-band gain = 1 - Influence * smoothed abs(sidechain). 0 dBFS key maps to full ducking.");
    refreshTrace();startTimerHz(30);
}
PhasePocketAudioProcessorEditor::~PhasePocketAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
void PhasePocketAudioProcessorEditor::resized(){spectrum.setBounds(620,25,128,40);amplitude.setBounds(752,25,140,40);influence.setBounds(712,136,156,158);smoothing.setBounds(712,362,156,158);}
void PhasePocketAudioProcessorEditor::refreshTrace(){
    PocketTrace t;while(processor.popTrace(t)){history[(size_t)cursor]=t;cursor=(cursor+1)%600;}
    amplitudeMode=processor.parameters.getRawParameterValue("mode")->load()>0.5f;
    spectrum.setToggleState(!amplitudeMode,juce::dontSendNotification);amplitude.setToggleState(amplitudeMode,juce::dontSendNotification);
    keyHold=juce::jmax(processor.keyPeak.load(),keyHold*0.88f);repaint();
}
void PhasePocketAudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(ui::bg);g.setColour(ui::border);g.drawHorizontalLine(85,24,896);
    ui::label(g,"PHASE POCKET",{28,20,365,37},28,ui::text);
    ui::label(g,"RAINLINE  /  DUAL-MODE SIDECHAIN",{30,57,400,20},13);
    ui::box(g,{24,104,650,208},ui::screen);ui::box(g,{24,328,650,236},ui::screen);ui::box(g,{690,104,206,460},ui::panel);
    ui::label(g,"GAIN HISTORY",{42,117,205,24},14,ui::text);
    ui::label(g,amplitudeMode?"WAVEFORM ENVELOPE":"SPECTRAL GAIN ESTIMATE",{265,117,388,24},13,ui::teal,juce::Justification::centredRight);
    const juce::Rectangle<float> plot(42,156,575,120);
    for(int i=0;i<=4;++i){g.setColour(ui::border.withAlpha(0.55f));g.drawHorizontalLine((int)(plot.getY()+i*30),plot.getX(),plot.getRight());}
    for(int i=0;i<=5;++i){g.setColour(ui::border.withAlpha(0.4f));g.drawVerticalLine((int)(plot.getX()+i*115),plot.getY(),plot.getBottom());}
    juce::Path gain;
    for(int i=0;i<600;++i){auto v=history[(size_t)((cursor+i)%600)];float x=plot.getX()+i*plot.getWidth()/599;float y=plot.getY()+(1-juce::jlimit(0.f,1.f,v.gain))*plot.getHeight();if(i==0)gain.startNewSubPath(x,y);else gain.lineTo(x,y);}
    auto fill=gain;fill.lineTo(plot.getRight(),plot.getY());fill.lineTo(plot.getX(),plot.getY());fill.closeSubPath();g.setColour(ui::teal.withAlpha(0.10f));g.fillPath(fill);
    g.setColour(ui::teal);g.strokePath(gain,juce::PathStrokeType(2));
    ui::label(g,"100%",{623,147,48,20},12);ui::label(g,"0%",{625,266,38,20},12);
    ui::label(g,"-500 ms",{42,281,130,20},12);ui::label(g,"NOW",{561,281,60,20},12,ui::muted,juce::Justification::centredRight);
    ui::label(g,"OSCILLOSCOPE",{42,339,180,24},14,ui::text);
    ui::label(g,"IN",{354,339,46,24},13,ui::muted);ui::label(g,"KEY",{410,339,48,24},13,ui::orange);ui::label(g,"OUT",{474,339,50,24},13,ui::teal);ui::label(g,"L / MONO",{551,339,108,24},12);
    const juce::Rectangle<float> scope(42,380,607,144);
    for(int i=0;i<=4;++i){g.setColour(ui::border.withAlpha(0.5f));g.drawHorizontalLine((int)(scope.getY()+i*36),scope.getX(),scope.getRight());}
    auto trace=[&](int kind,juce::Colour col){g.setColour(col);for(int i=0;i<600;++i){auto v=history[(size_t)((cursor+i)%600)];float lo=kind==0?v.inLo:kind==1?v.keyLo:v.outLo;float hi=kind==0?v.inHi:kind==1?v.keyHi:v.outHi;float x=scope.getX()+i*scope.getWidth()/599;float top=scope.getCentreY()-juce::jlimit(-1.f,1.f,hi)*68;float bottom=scope.getCentreY()-juce::jlimit(-1.f,1.f,lo)*68;g.drawLine(x,top,x,juce::jmax(top+0.6f,bottom),1);}};
    trace(0,ui::muted.withAlpha(0.35f));trace(1,ui::orange.withAlpha(0.8f));trace(2,ui::teal);
    ui::label(g,"Time-aligned input / key / output",{42,534,388,20},12);
    ui::label(g,"+/- 1.0",{561,534,86,20},12,ui::muted,juce::Justification::centredRight);
    ui::label(g,"INFLUENCE",{706,113,174,24},15,ui::text,juce::Justification::centred);
    ui::label(g,"Full action at 100%",{704,301,178,22},13,ui::muted,juce::Justification::centred);
    g.setColour(ui::border);g.drawHorizontalLine(334,710,876);
    ui::label(g,"SMOOTHING",{706,339,174,24},15,ui::text,juce::Justification::centred);
    ui::label(g,"Release / recovery",{704,527,178,22},13,ui::muted,juce::Justification::centred);
    g.setColour(keyHold>0.0001f?ui::teal:ui::muted);g.fillEllipse(28,588,7,7);
    ui::label(g,keyHold>0.0001f?"KEY ACTIVE":"NO KEY SIGNAL",{43,580,184,24},13);
    ui::label(g,amplitudeMode?"AMPLITUDE  /  FULL BAND":"SPECTRUM  /  20 Hz - 20 kHz",{244,580,330,24},13);
    const double sr=juce::jmax(1.0,processor.getSampleRate());
    ui::label(g,juce::String(1000*pocket::N/sr,1)+" ms latency  |  v0.2",{623,580,269,24},13,ui::muted,juce::Justification::centredRight);
}
