#include "PluginEditor.h"
namespace {
juce::Font uiFont(float size) {
#if JUCE_LINUX
    return juce::Font(juce::FontOptions("Liberation Sans",size,juce::Font::plain));
#else
    return juce::Font(juce::FontOptions("Arial",size,juce::Font::plain));
#endif
}
void text(juce::Graphics& g,const juce::String& s,juce::Rectangle<float> r,float size,juce::Colour c,int align=juce::Justification::centredLeft) {
    g.setColour(c);g.setFont(uiFont(size));g.drawText(s,r,align);
}
void stroke(juce::Graphics& g,const juce::Path& p,juce::Colour c,float width) {
    g.setColour(c);g.strokePath(p,juce::PathStrokeType(width,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
}
juce::String hz(double v) {return v>=1000?juce::String(v/1000,1)+" kHz":juce::String(juce::roundToInt(v))+" Hz";}
}
juce::Colour PocketLook::ink() const {return juce::Colour(dark?0xffdce5fc:0xff242527);}
juce::Colour PocketLook::muted() const {return juce::Colour(dark?0xff97a6c2:0xff6b6c70);}
juce::Font PocketLook::getTextButtonFont(juce::TextButton&,int) {return uiFont(16);}
void PocketLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down) {
    auto r=b.getLocalBounds().toFloat().reduced(2);bool selected=b.getToggleState();
    bool icon=b.getButtonText()=="theme"||b.getButtonText()=="power";
    auto top=juce::Colour(dark?(selected?0xff586fdb:0xff172130):(selected?0xff393b3e:0xfffafafa));
    auto bottom=juce::Colour(dark?(selected?0xff3246a1:0xff070d16):(selected?0xff17191b:0xffe3e4e6));
    if(hover||down){top=top.brighter(.12f);bottom=bottom.brighter(.1f);}
    g.setColour(juce::Colours::black.withAlpha(dark?.5f:.13f));g.fillRoundedRectangle(r.translated(0,3),icon?13.f:18.f);
    if(dark&&selected){g.setColour(top.withAlpha(.13f));g.fillRoundedRectangle(r.expanded(2),20);}
    g.setGradientFill(juce::ColourGradient(top,0,r.getY(),bottom,0,r.getBottom(),false));g.fillRoundedRectangle(r,icon?13.f:18.f);
    g.setColour(juce::Colour(dark?0xff34445b:0xffc2c3c6));g.drawRoundedRectangle(r,icon?13.f:18.f,.7f);
    if(b.hasKeyboardFocus(false)){g.setColour(ink());g.drawRoundedRectangle(r.reduced(2),12,1);}
}
void PocketLook::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool) {
    auto r=b.getLocalBounds().toFloat();const auto name=b.getButtonText();
    if(name!="theme"&&name!="power") {text(g,name,r,juce::jmin(16.f,r.getHeight()*.36f),b.getToggleState()?juce::Colours::white:ink(),juce::Justification::centred);return;}
    const auto c=r.getCentre();float s=juce::jmin(r.getWidth(),r.getHeight())/44.f;juce::Path p;
    if(name=="power") {
        p.addCentredArc(0,1,9,9,0,.65f,juce::MathConstants<float>::twoPi-.65f,true);
        p.startNewSubPath(0,-11);p.lineTo(0,-1);
    } else if(dark) {
        p.addEllipse(-4,-4,8,8);
        for(int i=0;i<8;++i){float a=float(i)*juce::MathConstants<float>::pi/4;p.startNewSubPath(7*std::cos(a),7*std::sin(a));p.lineTo(10*std::cos(a),10*std::sin(a));}
    } else {
        p.startNewSubPath(8,3);p.cubicTo(-4,8,-10,-4,-2,-9);p.cubicTo(-16,-8,-9,17,8,3);p.closeSubPath();
    }
    p.applyTransform(juce::AffineTransform::scale(s).translated(c.x,c.y));
    stroke(g,p,ink().withAlpha(name=="power"&&b.getToggleState()?.45f:1.f),1.7f*s);
}
void PocketLook::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float minPos,float maxPos,juce::Slider::SliderStyle style,juce::Slider&) {
    const float cy=float(y)+float(h)*.5f;float left=float(x),right=float(x+w);
    if(style==juce::Slider::TwoValueHorizontal){left=minPos;right=maxPos;}
    else right=pos;
    g.setColour(juce::Colour(dark?0xff172334:0xffc9cacc));g.fillRoundedRectangle(float(x),cy-3,float(w),6,3);
    const auto active=juce::Rectangle<float>(left,cy-3,juce::jmax(.1f,right-left),6);
    if(dark){g.setColour(juce::Colour(0xff3b94ee).withAlpha(.12f));g.fillRoundedRectangle(active.expanded(2),5);}
    g.setGradientFill(juce::ColourGradient(juce::Colour(dark?0xff4e78ff:0xff303236),left,cy,juce::Colour(dark?0xff35d6dc:0xff65676b),right+1,cy,false));g.fillRoundedRectangle(active,3);
    auto thumb=[&](float px){g.setColour(juce::Colours::black.withAlpha(.25f));g.fillEllipse(px-7,cy-4,14,14);g.setGradientFill(juce::ColourGradient(juce::Colours::white,px,cy-7,juce::Colour(dark?0xffbbd0ff:0xffd7d8db),px,cy+7,false));g.fillEllipse(px-6,cy-6,12,12);g.setColour(juce::Colour(dark?0xff638bda:0xff85878b));g.drawEllipse(px-6,cy-6,12,12,1);};
    if(style==juce::Slider::TwoValueHorizontal){thumb(minPos);thumb(maxPos);}else thumb(pos);
}
ModernDial::ModernDial(PocketLook& l,juce::String t,juce::String sub,juce::String u,juce::Colour a,bool inf):look(l),title(t),subtitle(sub),unit(u),accent(a),infinity(inf) {
    setSliderStyle(juce::Slider::RotaryVerticalDrag);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);setName(t);setWantsKeyboardFocus(true);
}
void ModernDial::paint(juce::Graphics& g) {
    const float s=float(getWidth())/185.f,r=65*s;const juce::Point<float> c(float(getWidth())*.5f,float(getHeight())*.5f);
    const auto face=juce::Rectangle<float>(2*r,2*r).withCentre(c);
    for(int i=6;i>0;--i){g.setColour(juce::Colours::black.withAlpha(look.dark?.065f:.022f));g.fillEllipse(face.expanded(float(i)*2*s).translated(0,float(i+2)*s));}
    auto top=juce::Colour(look.dark?0xff233349:0xfffafafa),bottom=juce::Colour(look.dark?0xff060b12:0xffd5d6d9);
    juce::ColourGradient surface(top,c.x-r,c.y-r,bottom,c.x+r,c.y+r,false);surface.addColour(.42,juce::Colour(look.dark?0xff142032:0xffeeeeef));g.setGradientFill(surface);g.fillEllipse(face);
    g.setColour(juce::Colour(look.dark?0xff344963:0xffbabcc0));g.drawEllipse(face,1*s);
    const float p=float(valueToProportionOfLength(getValue()));const float start=juce::MathConstants<float>::pi*1.25f,end=start+juce::MathConstants<float>::pi*1.5f*p;
    juce::Path track,arc;track.addCentredArc(c.x,c.y,r+8*s,r+8*s,0,start,juce::MathConstants<float>::pi*2.75f,true);
    stroke(g,track,juce::Colour(look.dark?0xff050a11:0xffc7c8ca),6*s);
    if(p>0){arc.addCentredArc(c.x,c.y,r+8*s,r+8*s,0,start,end,true);
        if(look.dark){stroke(g,arc,accent.withAlpha(.08f),16*s);stroke(g,arc,accent.withAlpha(.16f),10*s);}
        juce::ColourGradient gradient(look.dark?accent.brighter(.3f):juce::Colour(0xff292b2f),c.x-r,c.y+r,look.dark?accent.interpolatedWith(juce::Colour(0xffaa8aff),.4f):juce::Colour(0xff606268),c.x+r,c.y-r,false);
        g.setGradientFill(gradient);g.strokePath(arc,juce::PathStrokeType(5*s,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));}
    // JUCE arcs measure clockwise from 12 o'clock: x=sin(a), y=-cos(a).
    auto marker=juce::Point<float>(c.x+(r+8*s)*std::sin(end),c.y-(r+8*s)*std::cos(end));
    g.setColour(juce::Colours::black.withAlpha(.5f));g.fillEllipse(marker.x-7*s,marker.y-5*s,14*s,14*s);
    g.setGradientFill(juce::ColourGradient(juce::Colours::white,marker.x,marker.y-6*s,juce::Colour(look.dark?0xffc0d6ff:0xffd1d2d4),marker.x,marker.y+6*s,false));g.fillEllipse(marker.x-5.5f*s,marker.y-5.5f*s,11*s,11*s);
    text(g,title,{c.x-r,c.y-43*s,2*r,24*s},16*s,look.ink(),juce::Justification::centred);
    auto value=infinity&&p>.9995f?juce::String::fromUTF8("∞"):juce::String(getValue(),getInterval()<1?1:0)+(unit=="%"?"%":" ms");
    text(g,value,{c.x-r,c.y-18*s,2*r,40*s},unit=="ms"?26*s:30*s,look.ink(),juce::Justification::centred);
    text(g,subtitle,{c.x-r,c.y+25*s,2*r,22*s},12*s,look.muted(),juce::Justification::centred);
    if(hasKeyboardFocus(false)){g.setColour(look.muted());g.drawEllipse(face.expanded(3*s),1*s);}
}
PhasePocketAudioProcessorEditor::PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor& p):AudioProcessorEditor(&p),audioProcessor(p) {
    juce::PropertiesFile::Options o;o.applicationName="PhasePocket";o.filenameSuffix="settings";o.folderName="RainlineMusic";o.osxLibrarySubFolder="Application Support";
    preferences=std::make_unique<juce::PropertiesFile>(o);look.dark=preferences->getValue("phasePocket.ui.theme","dark")=="dark";
    setLookAndFeel(&look);setOpaque(true);setResizable(true,true);setResizeLimits(900,672,1800,1344);getConstrainer()->setFixedAspectRatio(75./56.);
    for(auto* component:std::initializer_list<juce::Component*>{&influence,&smoothing,&duration,&sustain,&sidechainRange,&midSide,&amplitude,&spectrum,&themeButton,&bypassButton,&resetFilter})addAndMakeVisible(component);
    influenceAttach=std::make_unique<SliderAttachment>(p.parameters,"amount",influence);smoothingAttach=std::make_unique<SliderAttachment>(p.parameters,"release",smoothing);durationAttach=std::make_unique<SliderAttachment>(p.parameters,"duration",duration);sustainAttach=std::make_unique<SliderAttachment>(p.parameters,"sustain",sustain);
    midSide.setSliderStyle(juce::Slider::LinearHorizontal);midSide.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);midSide.setDoubleClickReturnValue(true,0);
    msAttach=std::make_unique<SliderAttachment>(p.parameters,"msBalance",midSide);
    bypassButton.setClickingTogglesState(true);bypassAttach=std::make_unique<ButtonAttachment>(p.parameters,"bypass",bypassButton);
    amplitude.onClick=[this]{setMode(1);};spectrum.onClick=[this]{setMode(0);};themeButton.onClick=[this]{setThemeForPreview(!look.dark);preferences->setValue("phasePocket.ui.theme",look.dark?"dark":"light");preferences->saveIfNeeded();};
    sidechainRange.setSliderStyle(juce::Slider::TwoValueHorizontal);sidechainRange.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);sidechainRange.setRange(20,20000,1);sidechainRange.setSkewFactorFromMidPoint(std::sqrt(20.*20000.));
    lowAttach=std::make_unique<juce::ParameterAttachment>(*p.parameters.getParameter("scLow"),[this](float){syncRange();});
    highAttach=std::make_unique<juce::ParameterAttachment>(*p.parameters.getParameter("scHigh"),[this](float){syncRange();});lowAttach->sendInitialUpdate();highAttach->sendInitialUpdate();
    sidechainRange.onDragStart=[this]{rangeGesture=true;lowAttach->beginGesture();highAttach->beginGesture();};
    sidechainRange.onValueChange=[this]{float lo=float(sidechainRange.getMinValue()),hi=float(sidechainRange.getMaxValue());if(rangeGesture){lowAttach->setValueAsPartOfGesture(lo);highAttach->setValueAsPartOfGesture(hi);}else{lowAttach->setValueAsCompleteGesture(lo);highAttach->setValueAsCompleteGesture(hi);}};
    sidechainRange.onDragEnd=[this]{lowAttach->endGesture();highAttach->endGesture();rangeGesture=false;syncRange();};
    resetFilter.onClick=[this]{lowAttach->setValueAsCompleteGesture(20);highAttach->setValueAsCompleteGesture(20000);};
    influence.setTooltip("Processing depth");duration.setTooltip("Full-depth duration: 1 ms to infinity. Then a smooth transition to Sustain.");sustain.setTooltip("Sidechain level retained after the smooth decay");midSide.setTooltip("Left: Mid only. Centre: Mid and Side. Right: Side only.");sidechainRange.setTooltip("Sidechain detector low and high cutoff");bypassButton.setTooltip("Bypass processing");themeButton.setTooltip(look.dark?"Switch to light theme":"Switch to dark theme");
    int width=p.editorWidth.load();if(width<900||width>1800)width=1200;setSize(width,juce::roundToInt(width*56./75.));
    PocketTrace discard;while(p.popTrace(discard)){}p.editorOpen.store(true);timerCallback();startTimerHz(60);
}
PhasePocketAudioProcessorEditor::~PhasePocketAudioProcessorEditor(){stopTimer();audioProcessor.editorOpen.store(false);if(rangeGesture){lowAttach->endGesture();highAttach->endGesture();}setLookAndFeel(nullptr);}
void PhasePocketAudioProcessorEditor::setThemeForPreview(bool dark){look.dark=dark;themeButton.setTooltip(dark?"Switch to light theme":"Switch to dark theme");repaint();for(auto* c:getChildren())c->repaint();}
void PhasePocketAudioProcessorEditor::setMode(float v){auto* p=audioProcessor.parameters.getParameter("mode");p->beginChangeGesture();p->setValueNotifyingHost(v);p->endChangeGesture();amplitude.setToggleState(v>.5f,juce::dontSendNotification);spectrum.setToggleState(v<.5f,juce::dontSendNotification);repaint();}
void PhasePocketAudioProcessorEditor::syncRange(){if(rangeGesture)return;float a=audioProcessor.parameters.getRawParameterValue("scLow")->load(),b=audioProcessor.parameters.getRawParameterValue("scHigh")->load();sidechainRange.setMinAndMaxValues(juce::jmin(a,b),juce::jmax(a,b),juce::dontSendNotification);repaint();}
juce::Rectangle<int> PhasePocketAudioProcessorEditor::scaled(float x,float y,float w,float h) const {float s=float(getWidth())/1200;return{juce::roundToInt(x*s),juce::roundToInt(y*s),juce::roundToInt(w*s),juce::roundToInt(h*s)};}
void PhasePocketAudioProcessorEditor::resized(){amplitude.setBounds(scaled(810,33,126,44));spectrum.setBounds(scaled(936,33,126,44));themeButton.setBounds(scaled(1080,33,44,44));bypassButton.setBounds(scaled(1130,33,44,44));influence.setBounds(scaled(797,131,185,245));smoothing.setBounds(scaled(988,131,185,245));duration.setBounds(scaled(797,391,185,245));sustain.setBounds(scaled(988,391,185,245));sidechainRange.setBounds(scaled(68,744,1060,42));midSide.setBounds(scaled(116,807,220,32));resetFilter.setBounds(scaled(1040,696,105,33));audioProcessor.editorWidth.store(getWidth());}
void PhasePocketAudioProcessorEditor::drawPanel(juce::Graphics& g,juce::Rectangle<float> r){for(int i=5;i>0;--i){g.setColour(juce::Colours::black.withAlpha(look.dark?.055f:.015f));g.fillRoundedRectangle(r.expanded(float(i)).translated(0,float(i)*2),18+float(i));}g.setGradientFill(juce::ColourGradient(juce::Colour(look.dark?0xff182538:0xfffafafa),r.getX(),r.getY(),juce::Colour(look.dark?0xff060c14:0xffe8e8e9),r.getRight(),r.getBottom(),false));g.fillRoundedRectangle(r,18);g.setColour(juce::Colour(look.dark?0xff304259:0xffbfc0c2));g.drawRoundedRectangle(r,18,1);}
void PhasePocketAudioProcessorEditor::drawGraph(juce::Graphics& g,bool spectral){
    const juce::Rectangle<float> r(100,185,640,190);auto grid=juce::Colour(look.dark?0xff213249:0xff393b3d);g.setColour(juce::Colour(look.dark?0xff040a12:0xff1b1c1e));g.fillRoundedRectangle(r,8);
    text(g,spectral?"Spectral reduction":"Gain history",{50,140,380,32},22,look.ink());
    auto y=[&](float gain){return r.getBottom()-r.getHeight()*(spectral?std::pow(juce::jlimit(0.f,1.f,gain),.25f):juce::jlimit(0.f,1.f,gain));};
    if(spectral){for(float db:{0.f,-6.f,-12.f,-24.f,-48.f}){float yy=y(std::pow(10.f,db/20));g.setColour(grid);g.drawLine(r.getX(),yy,r.getRight(),yy);text(g,db==0?juce::String("0 dB"):juce::String(int(db)),{46,yy-10,50,20},13,look.muted());}for(float f:{20.f,100.f,1000.f,10000.f,20000.f}){float x=r.getX()+r.getWidth()*std::log(f/20)/std::log(1000.f);g.setColour(grid);g.drawLine(x,r.getY(),x,r.getBottom());text(g,f>=1000?juce::String(int(f/1000))+"k":juce::String(int(f)),{x-24,380,48,20},13,look.muted(),juce::Justification::centred);}text(g,"M    S",{665,142,80,28},14,look.muted(),juce::Justification::centredRight);}
    else{for(int i=0;i<=4;++i){g.setColour(grid);float yy=r.getY()+float(i)*r.getHeight()/4;g.drawLine(r.getX(),yy,r.getRight(),yy);float x=r.getX()+float(i)*r.getWidth()/4;g.drawLine(x,r.getY(),x,r.getBottom());}text(g,"100%",{45,175,52,20},13,look.muted());text(g,"0%",{55,362,42,20},13,look.muted());text(g,"-500 ms",{100,380,100,20},13,look.muted());text(g,"Now",{675,380,65,20},13,look.muted(),juce::Justification::centredRight);}
    juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(r.toNearestInt());
    if(spectral){for(int c=0;c<2;++c){auto& values=c?spectrumCurve.side:spectrumCurve.mid;juce::Path p;for(size_t i=0;i<values.size();++i){float x=r.getX()+r.getWidth()*float(i)/31,yy=y(std::isfinite(values[i])?values[i]:1);if(i==0)p.startNewSubPath(x,yy);else p.lineTo(x,yy);}auto colour=look.dark?juce::Colour(c?0xff33c9d2:0xff8298ff):juce::Colour(c?0xff8d9299:0xffe8e9ec);auto fill=p;fill.lineTo(r.getRight(),r.getY());fill.lineTo(r.getX(),r.getY());fill.closeSubPath();g.setColour(colour.withAlpha(.07f));g.fillPath(fill);stroke(g,p,colour,1.5f);}}
    else if(filled){double now=history[size_t((cursor+4095)%4096)].time;juce::Path p;bool started=false;for(int i=0;i<filled;++i){auto& v=history[size_t((cursor-filled+i+4096)%4096)];double age=now-v.time;if(age<0||age>.5||!std::isfinite(v.gain))continue;float x=r.getRight()-float(age*2)*r.getWidth(),yy=y(v.gain);if(!started){p.startNewSubPath(x,yy);started=true;}else p.lineTo(x,yy);}stroke(g,p,juce::Colour(0xffe4e7ef),1.7f);}
}
void PhasePocketAudioProcessorEditor::drawScope(juce::Graphics& g){
    const juce::Rectangle<float> r(50,452,690,166);text(g,"Oscilloscope",{50,410,340,30},22,look.ink());text(g,"OUT       KEY",{578,414,162,22},12,look.muted(),juce::Justification::centredRight);g.setColour(juce::Colour(look.dark?0xff040a12:0xff1b1c1e));g.fillRoundedRectangle(r,8);g.setColour(juce::Colour(look.dark?0xff253951:0xff414246));g.drawLine(r.getX(),r.getCentreY(),r.getRight(),r.getCentreY());text(g,"-1 s",{50,626,70,20},13,look.muted());text(g,"Now",{670,626,70,20},13,look.muted(),juce::Justification::centredRight);if(!filled)return;
    juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(r.toNearestInt());double now=history[size_t((cursor+4095)%4096)].time;
    // Aggregate min/max once per display column, not multiple glowing overlapping strokes.
    for(int kind=0;kind<2;++kind){std::array<float,690> low{},high{};std::array<bool,690> seen{};
        for(int i=0;i<filled;++i){auto& v=history[size_t((cursor-filled+i+4096)%4096)];double age=now-v.time;if(age<0||age>1)continue;int x=juce::jlimit(0,689,int((1-age)*689));float lo=kind?v.keyLo:v.outLo,hi=kind?v.keyHi:v.outHi;if(!std::isfinite(lo)||!std::isfinite(hi))continue;if(!seen[size_t(x)]){low[size_t(x)]=lo;high[size_t(x)]=hi;seen[size_t(x)]=true;}else{low[size_t(x)]=juce::jmin(low[size_t(x)],lo);high[size_t(x)]=juce::jmax(high[size_t(x)],hi);}}
        g.setColour(kind?juce::Colour(0xffe3e7ee).withAlpha(.8f):juce::Colour(look.dark?0xff3794d4:0xffadb3bc).withAlpha(.5f));for(size_t x=0;x<690;++x)if(seen[x]){float top=r.getCentreY()-juce::jlimit(-1.f,1.f,high[x])*80,bottom=r.getCentreY()-juce::jlimit(-1.f,1.f,low[x])*80;g.drawLine(r.getX()+float(x),top,r.getX()+float(x),juce::jmax(top+.5f,bottom),1.f);}
    }
}
void PhasePocketAudioProcessorEditor::paint(juce::Graphics& g){
    g.fillAll(juce::Colour(look.dark?0xff060b12:0xffececeb));juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::scale(float(getWidth())/1200));
    if(look.dark){g.setGradientFill(juce::ColourGradient(juce::Colour(0xff1d2b3e),560,0,juce::Colour(0xff04080e),560,850,true));g.fillRect(0,0,1200,896);for(int i=0;i<10;++i){juce::Path p;p.startNewSubPath(370+float(i)*22,-10);p.cubicTo(470+float(i)*19,80,430+float(i)*27,135,650+float(i)*30,170);stroke(g,p,juce::Colour(0xff52617e).withAlpha(.15f),.8f);}}
    text(g,"PHASE POCKET",{40,30,550,48},32,look.ink());g.setColour(juce::Colour(look.dark?0xff263347:0xffc5c6c8));g.drawLine(30,104,1170,104);
    drawPanel(g,{25,120,750,540});drawPanel(g,{790,120,385,540});drawPanel(g,{25,680,1150,185});
    drawGraph(g,audioProcessor.parameters.getRawParameterValue("mode")->load()<.5f);drawScope(g);
    text(g,"Sidechain filter",{50,698,190,28},19,look.ink());text(g,hz(sidechainRange.getMinValue()),{290,698,135,28},17,look.ink());text(g,hz(sidechainRange.getMaxValue()),{455,698,150,28},17,look.ink());
    text(g,"20",{66,786,50,18},12,look.muted());text(g,"20k",{1070,786,62,18},12,look.muted(),juce::Justification::centredRight);text(g,"M/S",{50,810,55,26},15,look.muted());
}
void PhasePocketAudioProcessorEditor::timerCallback(){PocketTrace v;while(audioProcessor.popTrace(v)){if(!std::isfinite(v.time))continue;if(filled&&v.time<=history[size_t((cursor+4095)%4096)].time)filled=0;history[size_t(cursor)]=v;cursor=(cursor+1)%4096;filled=juce::jmin(filled+1,4096);}SpectrumTrace c;while(audioProcessor.popSpectrum(c))spectrumCurve=c;bool amp=audioProcessor.parameters.getRawParameterValue("mode")->load()>.5f;amplitude.setToggleState(amp,juce::dontSendNotification);spectrum.setToggleState(!amp,juce::dontSendNotification);repaint();}
