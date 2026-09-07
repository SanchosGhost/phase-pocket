#include "PluginEditor.h"
namespace ui {
const juce::Colour bg{0xffececea},panel{0xfff8f8f6},screen{0xff191a1c},border{0xffb9bbbe},text{0xff202124},muted{0xff5b5e64},teal{0xfff4f4f2},orange{0xff96999f},ink{0xff191a1c},displayText{0xffdedee0},displayMuted{0xffa8abb0};
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
    setColour(juce::Slider::textBoxTextColourId,ui::text);setColour(juce::Slider::textBoxBackgroundColourId,ui::panel);
    setColour(juce::Slider::textBoxOutlineColourId,ui::border);setColour(juce::Slider::textBoxHighlightColourId,ui::ink.withAlpha(0.15f));
    setColour(juce::TextButton::buttonColourId,ui::panel);setColour(juce::TextButton::buttonOnColourId,ui::ink);
    setColour(juce::TextButton::textColourOffId,ui::muted);setColour(juce::TextButton::textColourOnId,ui::teal);
}
void PocketLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down){
    auto r=b.getLocalBounds().toFloat().reduced(5,5);
    juce::DropShadow(juce::Colours::black.withAlpha(down?0.10f:0.20f),7,{0,3}).drawForRectangle(g,r.toNearestInt());
    if(down)r.translate(0,1);bool on=b.getToggleState();
    auto top=on?juce::Colour(0xff3a3c40):juce::Colour(0xffffffff);
    auto bottom=on?juce::Colour(0xff18191c):juce::Colour(0xffe4e5e6);
    if(hover){top=top.brighter(0.05f);bottom=bottom.brighter(0.05f);}
    g.setGradientFill(juce::ColourGradient(top,r.getTopLeft(),bottom,r.getBottomLeft(),false));g.fillRoundedRectangle(r,8);
    g.setColour(on?juce::Colour(0xff4c4e52):juce::Colour(0xffbfc1c4));g.drawRoundedRectangle(r,8,1);
}
juce::Font PocketLook::getTextButtonFont(juce::TextButton&,int){return ui::font(14,true);}
void PocketLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider&){
    const float side=(float)juce::jmin(w,h)-18;
    auto r=juce::Rectangle<float>(side,side).withCentre({x+w*0.5f,y+h*0.5f});auto centre=r.getCentre();const float radius=side*0.5f;
    juce::Path track,arc;track.addCentredArc(centre.x,centre.y,radius,radius,0,start,end,true);
    g.setColour(ui::border);g.strokePath(track,juce::PathStrokeType(3));
    arc.addCentredArc(centre.x,centre.y,radius,radius,0,start,start+value*(end-start),true);
    g.setColour(ui::ink);g.strokePath(arc,juce::PathStrokeType(3,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    auto face=r.reduced(9);juce::Path shadow;shadow.addEllipse(face);juce::DropShadow(juce::Colours::black.withAlpha(0.23f),9,{0,4}).drawForPath(g,shadow);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffffff),face.getTopLeft(),juce::Colour(0xffd6d8da),face.getBottomRight(),false));g.fillEllipse(face);
    g.setColour(juce::Colour(0xffafb2b6));g.drawEllipse(face,1);
    const float a=start+value*(end-start);
    const auto p=centre+juce::Point<float>(std::sin(a),-std::cos(a))*(radius-17),q=centre+juce::Point<float>(std::sin(a),-std::cos(a))*(radius-29);
    g.setColour(ui::text);g.drawLine({p,q},3);
}
PhasePocketAudioProcessorEditor::PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor& p):AudioProcessorEditor(&p),processor(p),keyBand(p.parameters){
    setLookAndFeel(&look);setSize(920,684);
    for(auto* s:{&influence,&smoothing}){s->setSliderStyle(juce::Slider::RotaryVerticalDrag);s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,140,30);s->setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);addAndMakeVisible(*s);s->setColour(juce::Slider::textBoxTextColourId,ui::ink);s->setColour(juce::Slider::textBoxBackgroundColourId,ui::panel);}
    influence.setTextValueSuffix(" %");smoothing.setTextValueSuffix(" ms");influence.setName("Influence");smoothing.setName("Smoothing");
    influence.setTooltip("0%: dry. 100%: normal depth. Up to 150%: extra attenuation, never negative gain.");
    influence.setDoubleClickReturnValue(true,100);addAndMakeVisible(keyBand);
    smoothing.setDoubleClickReturnValue(true,40);
    smoothing.setTooltip("Release time constant. 0 ms follows the key most closely; larger values extend the carve.");
    amountAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,"amount",influence);
    smoothAttach=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,"release",smoothing);
    for(auto* b:{&spectrum,&amplitude})addAndMakeVisible(*b);
    auto choose=[this](int index){auto* m=processor.parameters.getParameter("mode");m->beginChangeGesture();m->setValueNotifyingHost(m->convertTo0to1((float)index));m->endChangeGesture();refreshTrace();};
    spectrum.onClick=[choose]{choose(0);};amplitude.onClick=[choose]{choose(1);};
    spectrum.setTooltip("Phase-aware spectral budget, 20 Hz to 20 kHz (up to Nyquist).");
    amplitude.setTooltip("Full-band gain = 1 - Influence * smoothed abs(sidechain). Filtered 0 dBFS key maps to full ducking at 100%.");
    refreshTrace();startTimerHz(30);
}
PhasePocketAudioProcessorEditor::~PhasePocketAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
void PhasePocketAudioProcessorEditor::resized(){spectrum.setBounds(612,19,136,52);amplitude.setBounds(748,19,148,52);keyBand.setBounds(24,578,650,88);influence.setBounds(712,136,156,158);smoothing.setBounds(712,362,156,158);}
void PhasePocketAudioProcessorEditor::refreshTrace(){
    PocketTrace t;while(processor.popTrace(t)){history[(size_t)cursor]=t;cursor=(cursor+1)%600;}
    amplitudeMode=processor.parameters.getRawParameterValue("mode")->load()>0.5f;
    spectrum.setToggleState(!amplitudeMode,juce::dontSendNotification);amplitude.setToggleState(amplitudeMode,juce::dontSendNotification);keyBand.sync();repaint();
}
void PhasePocketAudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(ui::bg);g.setColour(ui::border);g.drawHorizontalLine(85,24,896);
    ui::label(g,"PHASE POCKET",{28,20,365,37},28,ui::text);
    ui::label(g,"RAINLINE  /  DUAL-MODE SIDECHAIN",{30,57,400,20},13);
    ui::box(g,{24,104,650,208},ui::screen);ui::box(g,{24,328,650,236},ui::screen);ui::box(g,{690,104,206,460},ui::panel);
    ui::label(g,"GAIN HISTORY",{42,117,205,24},14,ui::displayText);
    ui::label(g,amplitudeMode?"WAVEFORM ENVELOPE":"SPECTRAL GAIN ESTIMATE",{265,117,388,24},13,ui::teal,juce::Justification::centredRight);
    const juce::Rectangle<float> plot(42,156,575,120);
    for(int i=0;i<=4;++i){g.setColour(juce::Colour(0xff37393d));g.drawHorizontalLine((int)(plot.getY()+i*30),plot.getX(),plot.getRight());}
    for(int i=0;i<=5;++i){g.setColour(juce::Colour(0xff303236));g.drawVerticalLine((int)(plot.getX()+i*115),plot.getY(),plot.getBottom());}
    juce::Path gain;
    for(int i=0;i<600;++i){auto v=history[(size_t)((cursor+i)%600)];float x=plot.getX()+i*plot.getWidth()/599,y=plot.getY()+(1-juce::jlimit(0.f,1.f,v.gain))*plot.getHeight();if(i==0)gain.startNewSubPath(x,y);else gain.lineTo(x,y);}
    auto fill=gain;fill.lineTo(plot.getRight(),plot.getY());fill.lineTo(plot.getX(),plot.getY());fill.closeSubPath();g.setColour(ui::teal.withAlpha(0.10f));g.fillPath(fill);
    g.setColour(ui::teal);g.strokePath(gain,juce::PathStrokeType(2));
    ui::label(g,"100%",{623,147,48,20},12,ui::displayMuted);ui::label(g,"0%",{625,266,38,20},12,ui::displayMuted);
    ui::label(g,"-500 ms",{42,281,130,20},12,ui::displayMuted);ui::label(g,"NOW",{561,281,60,20},12,ui::displayMuted,juce::Justification::centredRight);
    ui::label(g,"OSCILLOSCOPE",{42,339,180,24},14,ui::displayText);
    ui::label(g,"IN",{354,339,46,24},13,ui::displayMuted);ui::label(g,"KEY",{410,339,48,24},13,ui::orange);ui::label(g,"OUT",{474,339,50,24},13,ui::teal);ui::label(g,"L / MONO",{551,339,108,24},12,ui::displayMuted);
    const juce::Rectangle<float> scope(42,380,607,144);
    for(int i=0;i<=4;++i){g.setColour(juce::Colour(0xff37393d));g.drawHorizontalLine((int)(scope.getY()+i*36),scope.getX(),scope.getRight());}
    auto trace=[&](int kind,juce::Colour col){g.setColour(col);for(int i=0;i<600;++i){auto v=history[(size_t)((cursor+i)%600)];float lo=kind==0?v.inLo:kind==1?v.keyLo:v.outLo,hi=kind==0?v.inHi:kind==1?v.keyHi:v.outHi,x=scope.getX()+i*scope.getWidth()/599,top=scope.getCentreY()-juce::jlimit(-1.f,1.f,hi)*68,bottom=scope.getCentreY()-juce::jlimit(-1.f,1.f,lo)*68;g.drawLine(x,top,x,juce::jmax(top+0.6f,bottom),1);}};
    trace(0,ui::displayMuted.withAlpha(0.3f));trace(1,ui::orange.withAlpha(0.8f));trace(2,ui::teal);
    ui::label(g,"Time-aligned input / filtered key / output",{42,534,470,20},12,ui::displayMuted);
    ui::label(g,"+/- 1.0",{561,534,86,20},12,ui::displayMuted,juce::Justification::centredRight);
    ui::label(g,"INFLUENCE",{706,113,174,24},15,ui::text,juce::Justification::centred);
    ui::label(g,"100% normal / 150% deep",{704,301,178,22},13,ui::muted,juce::Justification::centred);
    g.setColour(ui::border);g.drawHorizontalLine(334,710,876);
    ui::label(g,"SMOOTHING",{706,339,174,24},15,ui::text,juce::Justification::centred);
    ui::label(g,"Release / recovery",{704,527,178,22},13,ui::muted,juce::Justification::centred);
    const double sr=processor.getSampleRate()>0?processor.getSampleRate():48000;
    ui::label(g,"PHASE POCKET / 0.3",{702,585,190,22},12,ui::muted,juce::Justification::centred);
    ui::label(g,juce::String(1000*pocket::N/sr,1)+" ms latency",{702,610,190,22},13,ui::muted,juce::Justification::centred);
    ui::label(g,"STEREO LINKED",{702,636,190,22},12,ui::muted,juce::Justification::centred);
}
KeyBand::KeyBand(juce::AudioProcessorValueTreeState& p):state(p){setName("Sidechain frequency range");setWantsKeyboardFocus(true);sync();}
float KeyBand::position(float hz) const {return 92.f+std::log(hz/20.f)/std::log(1000.f)*(getWidth()-184.f);}
void KeyBand::sync(){if(active<0){low=state.getRawParameterValue("scLow")->load();high=state.getRawParameterValue("scHigh")->load();if(low>high)std::swap(low,high);}repaint();}
void KeyBand::paint(juce::Graphics& g){
    ui::box(g,getLocalBounds().toFloat().reduced(0.5f),ui::panel);
    ui::label(g,"SIDECHAIN FILTER",{16,8,210,24},13,ui::text);
    const bool full=low<=20.01f && high>=19999;
    ui::label(g,full?"FULL RANGE":"FILTERED KEY",{232,8,218,24},12,ui::muted,juce::Justification::centred);
    auto reset=juce::Rectangle<float>((float)getWidth()-126,8,110,24);
    g.setColour(juce::Colour(0xffe4e5e6));g.fillRoundedRectangle(reset,5);
    ui::label(g,"RESET",reset.toNearestInt(),12,ui::text,juce::Justification::centred);
    const float left=position(low),right=position(high),y=55;
    g.setColour(ui::border);g.drawLine(92,y,(float)getWidth()-92,y,4);
    g.setColour(ui::ink);g.drawLine(left,y,right,y,4);
    for(float hz:{20.f,100.f,1000.f,10000.f,20000.f}){float x=position(hz);g.setColour(ui::border);g.drawLine(x,64,x,69,1);}
    for(float x:{left,right}){
        auto thumb=juce::Rectangle<float>(x-8,43,16,24);
        juce::DropShadow(juce::Colours::black.withAlpha(0.18f),5,{0,2}).drawForRectangle(g,thumb.toNearestInt());
        g.setGradientFill(juce::ColourGradient(juce::Colours::white,thumb.getTopLeft(),juce::Colour(0xffdedfe1),thumb.getBottomLeft(),false));g.fillRoundedRectangle(thumb,5);
        g.setColour(ui::muted);g.drawRoundedRectangle(thumb,5,1);g.drawLine(x,49,x,61,1);
    }
    auto hz=[](float v){return v>=1000?juce::String(v/1000,1)+"k":juce::String((int)v)+" Hz";};
    ui::label(g,hz(low),{12,43,72,24},14,ui::text,juce::Justification::centred);
    ui::label(g,hz(high),{getWidth()-84,43,72,24},14,ui::text,juce::Justification::centred);
}
void KeyBand::apply(float x){
    float t=juce::jlimit(0.f,1.f,(x-92.f)/(getWidth()-184.f)),hz=20*std::pow(1000.f,t);
    if(active==0)low=juce::jlimit(20.f,high,hz);else high=juce::jlimit(low,20000.f,hz);
    auto* p=state.getParameter(active==0?"scLow":"scHigh");p->setValueNotifyingHost(p->convertTo0to1(active==0?low:high));repaint();
}
void KeyBand::mouseDown(const juce::MouseEvent& e){
    if(e.position.y<34 && e.position.x>getWidth()-130){mouseDoubleClick(e);return;}if(e.position.y<34)return;
    active=std::abs(e.position.x-position(low))<std::abs(e.position.x-position(high))?0:1;
    state.getParameter(active==0?"scLow":"scHigh")->beginChangeGesture();apply(e.position.x);
}
void KeyBand::mouseDrag(const juce::MouseEvent& e){if(active>=0)apply(e.position.x);}
void KeyBand::mouseUp(const juce::MouseEvent&){if(active>=0)state.getParameter(active==0?"scLow":"scHigh")->endChangeGesture();active=-1;sync();}
void KeyBand::mouseDoubleClick(const juce::MouseEvent&){
    if(active>=0){state.getParameter(active==0?"scLow":"scHigh")->endChangeGesture();active=-1;}
    for(auto id:{"scLow","scHigh"}){auto* p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(juce::String(id)=="scLow"?20.f:20000.f));p->endChangeGesture();}sync();
}
