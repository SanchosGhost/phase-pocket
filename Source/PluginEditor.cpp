#include "PluginEditor.h"
namespace {
juce::Font uiFont(float size){
    static const juce::String family=[] {
        const auto fonts=juce::Font::findAllTypefaceNames();
#if JUCE_WINDOWS
        for(auto name:{"Segoe UI Variable Text","Segoe UI Variable","Segoe UI"})if(fonts.contains(name))return juce::String(name);
#elif JUCE_MAC
        for(auto name:{"SF Pro Text","SF Pro",".AppleSystemUIFont"})if(fonts.contains(name))return juce::String(name);
#else
        if(fonts.contains("Liberation Sans"))return juce::String("Liberation Sans");
#endif
        return juce::Font::getDefaultSansSerifFontName();
    }();
    juce::Font font(juce::FontOptions(family,size,juce::Font::plain));
#if JUCE_WINDOWS
    static const juce::String style=[] {auto styles=juce::Font::findAllTypefaceStyles(family);for(auto s:{"Semilight Text","Semilight","SemiLight"})if(styles.contains(s))return juce::String(s);return juce::String();}();
    if(style.isNotEmpty())font.setTypefaceStyle(style);
#endif
    return font;
}
void text(juce::Graphics& g,const juce::String& s,juce::Rectangle<float> r,float size,juce::Colour c,int align=juce::Justification::centredLeft){g.setColour(c);g.setFont(uiFont(size));g.drawText(s,r,align);}
void stroke(juce::Graphics& g,const juce::Path& p,juce::Colour c,float width){g.setColour(c);g.strokePath(p,juce::PathStrokeType(width,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));}
juce::String hz(double v){return v>=1000?juce::String(v/1000,1)+" kHz":juce::String(juce::roundToInt(v))+" Hz";}
}
juce::Colour PocketLook::colour(juce::uint32 d,juce::uint32 l) const {auto c=juce::Colour(dark?d:l);return disabled?c.withSaturation(0):c;}
juce::Colour PocketLook::ink() const {return colour(0xffdce5fc,0xff242527);}
juce::Colour PocketLook::muted() const {return colour(0xff97a6c2,0xff6b6c70);}
juce::Font PocketLook::getTextButtonFont(juce::TextButton&,int){return uiFont(15);}
void PocketLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool){auto r=b.getLocalBounds().toFloat().reduced(2);g.setColour(juce::Colours::black.withAlpha(dark?.35f:.12f));g.fillRoundedRectangle(r.translated(0,2),9);g.setGradientFill(juce::ColourGradient(colour(hover?0xff26354c:0xff172130,0xfffafafa),0,r.getY(),colour(0xff070d16,0xffdedfe1),0,r.getBottom(),false));g.fillRoundedRectangle(r,9);g.setColour(colour(0xff34445b,0xffc2c3c6));g.drawRoundedRectangle(r,9,.7f);}
void PocketLook::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool){auto r=b.getLocalBounds().toFloat();auto name=b.getButtonText();if(name=="Reset"){text(g,name,r,14,ink(),juce::Justification::centred);return;}auto c=r.getCentre();float s=juce::jmin(r.getWidth(),r.getHeight())/40;juce::Path p;
    if(name=="power"){p.addCentredArc(0,1,8,8,0,.65f,juce::MathConstants<float>::twoPi-.65f,true);p.startNewSubPath(0,-10);p.lineTo(0,-1);}
    else if(name=="panel"){float dir=b.getToggleState()?-1.f:1.f;for(float y:{-4.f,3.f}){p.startNewSubPath(-5,y-2*dir);p.lineTo(0,y+2*dir);p.lineTo(5,y-2*dir);}}
    else if(dark){p.addEllipse(-4,-4,8,8);for(int i=0;i<8;++i){float a=float(i)*juce::MathConstants<float>::pi/4;p.startNewSubPath(7*std::cos(a),7*std::sin(a));p.lineTo(10*std::cos(a),10*std::sin(a));}}
    else{p.startNewSubPath(8,3);p.cubicTo(-4,8,-10,-4,-2,-9);p.cubicTo(-16,-8,-9,17,8,3);p.closeSubPath();}
    p.applyTransform(juce::AffineTransform::scale(s).translated(c.x,c.y));stroke(g,p,ink(),1.7f*s);
}
void PocketLook::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float minPos,float maxPos,juce::Slider::SliderStyle style,juce::Slider&){float cy=float(y)+float(h)*.5f,left=style==juce::Slider::TwoValueHorizontal?minPos:float(x),right=style==juce::Slider::TwoValueHorizontal?maxPos:pos;g.setColour(colour(0xff172334,0xffc9cacc));g.fillRoundedRectangle(float(x),cy-3,float(w),6,3);g.setGradientFill(juce::ColourGradient(colour(0xff4e78ff,0xff303236),left,cy,colour(0xff35d6dc,0xff65676b),right+1,cy,false));g.fillRoundedRectangle(left,cy-3,juce::jmax(.1f,right-left),6,3);auto thumb=[&](float px){g.setColour(colour(0xffdfebff,0xfff2f2f3));g.fillEllipse(px-6,cy-6,12,12);g.setColour(colour(0xff638bda,0xff85878b));g.drawEllipse(px-6,cy-6,12,12,1);};if(style==juce::Slider::TwoValueHorizontal){thumb(minPos);thumb(maxPos);}else thumb(pos);}
ModernDial::ModernDial(PocketLook& l,juce::String t,juce::String sub,juce::String u,juce::uint32 a,bool inf):look(l),title(t),subtitle(sub),unit(u),accent(a),infinity(inf){setSliderStyle(juce::Slider::RotaryVerticalDrag);setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);setName(t);setWantsKeyboardFocus(true);}
void ModernDial::paint(juce::Graphics& g){float s=float(getWidth())/200,r=77*s;juce::Point<float> c(float(getWidth())*.5f,float(getHeight())*.5f);auto face=juce::Rectangle<float>(2*r,2*r).withCentre(c);
    for(int i=5;i>0;--i){g.setColour(juce::Colours::black.withAlpha(look.dark?.06f:.02f));g.fillEllipse(face.expanded(float(i)*2*s).translated(0,float(i+2)*s));}
    g.setGradientFill(juce::ColourGradient(look.colour(0xff233349,0xfffafafa),c.x-r,c.y-r,look.colour(0xff060b12,0xffd5d6d9),c.x+r,c.y+r,false));g.fillEllipse(face);g.setColour(look.colour(0xff344963,0xffbabcc0));g.drawEllipse(face,s);
    float p=float(valueToProportionOfLength(getValue())),start=juce::MathConstants<float>::pi*1.25f,end=start+juce::MathConstants<float>::pi*1.5f*p;juce::Path track,arc;track.addCentredArc(c.x,c.y,r+8*s,r+8*s,0,start,juce::MathConstants<float>::pi*2.75f,true);stroke(g,track,look.colour(0xff050a11,0xffc7c8ca),6*s);
    if(p>0){arc.addCentredArc(c.x,c.y,r+8*s,r+8*s,0,start,end,true);auto a=look.colour(accent,0xff303235);if(look.dark){stroke(g,arc,a.withAlpha(.09f),14*s);stroke(g,arc,a.withAlpha(.15f),9*s);}g.setGradientFill(juce::ColourGradient(a.brighter(.2f),c.x-r,c.y+r,look.colour(0xff8c86ed,0xff67696d),c.x+r,c.y-r,false));g.strokePath(arc,juce::PathStrokeType(5*s,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));}
    auto marker=juce::Point<float>(c.x+(r+8*s)*std::sin(end),c.y-(r+8*s)*std::cos(end));g.setColour(look.colour(0xffdeebff,0xfff3f3f4));g.fillEllipse(marker.x-5.5f*s,marker.y-5.5f*s,11*s,11*s);
    text(g,title,{c.x-r,c.y-46*s,2*r,25*s},18*s,look.ink(),juce::Justification::centred);auto value=infinity&&p>.9995f?juce::String::fromUTF8("∞"):juce::String(getValue(),unit=="%"?1:0)+(unit=="%"?"%":" ms");text(g,value,{c.x-r,c.y-19*s,2*r,43*s},unit=="%"?33*s:30*s,look.ink(),juce::Justification::centred);text(g,subtitle,{c.x-r,c.y+28*s,2*r,23*s},13*s,look.muted(),juce::Justification::centred);
}
PhasePocketAudioProcessorEditor::PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor& p):AudioProcessorEditor(&p),audioProcessor(p){
    juce::PropertiesFile::Options o;o.applicationName="PhasePocket";o.filenameSuffix="settings";o.folderName="RainlineMusic";o.osxLibrarySubFolder="Application Support";preferences=std::make_unique<juce::PropertiesFile>(o);look.dark=preferences->getValue("phasePocket.ui.theme","dark")=="dark";
    setLookAndFeel(&look);setOpaque(true);setResizable(true,true);
    for(auto* c:std::initializer_list<juce::Component*>{&influence,&duration,&sidechainRange,&midSide,&themeButton,&bypassButton,&panelButton,&resetFilter})addAndMakeVisible(c);
    influenceAttach=std::make_unique<SliderAttachment>(p.parameters,"amount",influence);durationAttach=std::make_unique<SliderAttachment>(p.parameters,"duration",duration);
    midSide.setSliderStyle(juce::Slider::LinearHorizontal);midSide.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);midSide.setDoubleClickReturnValue(true,0);msAttach=std::make_unique<SliderAttachment>(p.parameters,"msBalance",midSide);
    bypassButton.setClickingTogglesState(true);bypassAttach=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"bypass",bypassButton);bypassButton.onClick=[this]{look.disabled=bypassButton.getToggleState()||audioProcessor.displayBypass.load();repaint();for(auto* c:getChildren())c->repaint();};
    themeButton.onClick=[this]{setThemeForPreview(!look.dark);preferences->setValue("phasePocket.ui.theme",look.dark?"dark":"light");preferences->saveIfNeeded();};panelButton.onClick=[this]{setPanelExpanded(!expanded);};
    sidechainRange.setSliderStyle(juce::Slider::TwoValueHorizontal);sidechainRange.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);sidechainRange.setRange(20,20000,1);sidechainRange.setSkewFactorFromMidPoint(std::sqrt(20.*20000.));
    lowAttach=std::make_unique<juce::ParameterAttachment>(*p.parameters.getParameter("scLow"),[this](float){syncRange();});highAttach=std::make_unique<juce::ParameterAttachment>(*p.parameters.getParameter("scHigh"),[this](float){syncRange();});lowAttach->sendInitialUpdate();highAttach->sendInitialUpdate();
    sidechainRange.onDragStart=[this]{rangeGesture=true;lowAttach->beginGesture();highAttach->beginGesture();};sidechainRange.onValueChange=[this]{float a=float(sidechainRange.getMinValue()),b=float(sidechainRange.getMaxValue());if(rangeGesture){lowAttach->setValueAsPartOfGesture(a);highAttach->setValueAsPartOfGesture(b);}else{lowAttach->setValueAsCompleteGesture(a);highAttach->setValueAsCompleteGesture(b);}};sidechainRange.onDragEnd=[this]{lowAttach->endGesture();highAttach->endGesture();rangeGesture=false;syncRange();};resetFilter.onClick=[this]{lowAttach->setValueAsCompleteGesture(20);highAttach->setValueAsCompleteGesture(20000);};
    duration.setTooltip("Total key duration: 1 ms to infinity; last 20% fades to zero");influence.setTooltip("Ducking depth");bypassButton.setTooltip("Enable / bypass processing");panelButton.setTooltip("Show sidechain filter and M/S");
    int width=p.editorWidth.load();if(width<800||width>1500)width=preferences->getIntValue("phasePocket.ui.width",960);width=juce::jlimit(800,1500,width);
    // Ignore constructor resize callbacks until saved dimensions have been restored.
    setResizeLimits(800,530,1500,1250);getConstrainer()->setFixedAspectRatio(960./636.);setSize(width,juce::roundToInt(width*636./960.));
    sidechainRange.setVisible(false);midSide.setVisible(false);resetFilter.setVisible(false);ready=true;p.editorWidth.store(width);
    PocketTrace discard;while(p.popTrace(discard)){}p.editorOpen.store(true);timerCallback();startTimerHz(60);
}
PhasePocketAudioProcessorEditor::~PhasePocketAudioProcessorEditor(){stopTimer();saveSize();audioProcessor.editorOpen.store(false);if(rangeGesture){lowAttach->endGesture();highAttach->endGesture();}setLookAndFeel(nullptr);}
void PhasePocketAudioProcessorEditor::saveSize(){if(!ready||!preferences)return;audioProcessor.editorWidth.store(getWidth());preferences->setValue("phasePocket.ui.width",getWidth());preferences->saveIfNeeded();resizeStamp=0;}
void PhasePocketAudioProcessorEditor::setThemeForPreview(bool dark){look.dark=dark;themeButton.setTooltip(dark?"Switch to light theme":"Switch to dark theme");repaint();for(auto* c:getChildren())c->repaint();}
void PhasePocketAudioProcessorEditor::setPanelExpanded(bool open){expanded=open;panelButton.setToggleState(open,juce::dontSendNotification);panelButton.setTooltip(open?"Hide sidechain filter and M/S":"Show sidechain filter and M/S");sidechainRange.setVisible(open);midSide.setVisible(open);resetFilter.setVisible(open);double height=open?800.:636.;getConstrainer()->setFixedAspectRatio(960./height);setSize(getWidth(),juce::roundToInt(getWidth()*height/960.));resized();repaint();}
void PhasePocketAudioProcessorEditor::syncRange(){if(rangeGesture)return;float a=audioProcessor.parameters.getRawParameterValue("scLow")->load(),b=audioProcessor.parameters.getRawParameterValue("scHigh")->load();sidechainRange.setMinAndMaxValues(juce::jmin(a,b),juce::jmax(a,b),juce::dontSendNotification);repaint();}
juce::Rectangle<int> PhasePocketAudioProcessorEditor::scaled(float x,float y,float w,float h) const {float s=float(getWidth())/960;return{juce::roundToInt(x*s),juce::roundToInt(y*s),juce::roundToInt(w*s),juce::roundToInt(h*s)};}
void PhasePocketAudioProcessorEditor::resized(){themeButton.setBounds(scaled(838,22,40,40));bypassButton.setBounds(scaled(888,22,40,40));influence.setBounds(scaled(725,104,200,220));duration.setBounds(scaled(725,350,200,220));panelButton.setBounds(scaled(24,590,36,30));sidechainRange.setBounds(scaled(52,689,842,36));midSide.setBounds(scaled(102,739,210,30));resetFilter.setBounds(scaled(837,649,75,29));if(ready){audioProcessor.editorWidth.store(getWidth());resizeStamp=juce::Time::getMillisecondCounterHiRes();}}
void PhasePocketAudioProcessorEditor::panel(juce::Graphics& g,juce::Rectangle<float> r){for(int i=4;i>0;--i){g.setColour(juce::Colours::black.withAlpha(look.dark?.05f:.012f));g.fillRoundedRectangle(r.expanded(float(i)).translated(0,float(i)*2),16+float(i));}g.setGradientFill(juce::ColourGradient(look.colour(0xff182538,0xfffafafa),r.getX(),r.getY(),look.colour(0xff060c14,0xffe8e8e9),r.getRight(),r.getBottom(),false));g.fillRoundedRectangle(r,16);g.setColour(look.colour(0xff304259,0xffbfc0c2));g.drawRoundedRectangle(r,16,1);}
void PhasePocketAudioProcessorEditor::graph(juce::Graphics& g,juce::Rectangle<float> box,bool gain){
    g.setColour(look.colour(0xff050b13,0xff1b1c1e));g.fillRoundedRectangle(box,12);auto labelColour=look.colour(0xffdce5fc,0xffe4e5e7);auto secondary=look.colour(0xff8fa2c0,0xffb1b2b5);auto grid=look.colour(0xff223349,0xff393b3e);
    text(g,gain?"GAIN HISTORY":"OSCILLOSCOPE",{box.getX()+18,box.getY()+12,220,24},13,labelColour);text(g,gain?"WAVEFORM ENVELOPE":"OUT     KEY",{box.getRight()-240,box.getY()+12,216,24},11,secondary,juce::Justification::centredRight);
    const juce::Rectangle<float> plot(box.getX()+18,box.getY()+47,box.getWidth()-70,box.getHeight()-87);
    for(int i=0;i<=4;++i){g.setColour(grid);float y=plot.getY()+float(i)*plot.getHeight()/4;g.drawLine(plot.getX(),y,plot.getRight(),y);float x=plot.getX()+float(i)*plot.getWidth()/4;g.drawLine(x,plot.getY(),x,plot.getBottom());}
    if(gain){text(g,"100%",{plot.getRight()+7,plot.getY()-9,43,20},11,labelColour);text(g,"0%",{plot.getRight()+7,plot.getBottom()-10,43,20},11,labelColour);}else{text(g,"+1",{plot.getRight()+7,plot.getY()-9,40,20},11,secondary);text(g,"-1",{plot.getRight()+7,plot.getBottom()-10,40,20},11,secondary);}
    text(g,gain?"-500 ms":"-1 s",{plot.getX(),box.getBottom()-29,100,20},11,secondary);text(g,"NOW",{plot.getRight()-70,box.getBottom()-29,70,20},11,secondary,juce::Justification::centredRight);
    if(!filled)return;double now=history[size_t((cursor+4095)%4096)].time;juce::Graphics::ScopedSaveState clip(g);g.reduceClipRegion(plot.toNearestInt());
    if(gain){juce::Path p;bool started=false;for(int i=0;i<filled;++i){auto& v=history[size_t((cursor-filled+i+4096)%4096)];double age=now-v.time;if(age<0||age>.5||!std::isfinite(v.gain))continue;float x=plot.getRight()-float(age*2)*plot.getWidth(),y=plot.getBottom()-juce::jlimit(0.f,1.f,v.gain)*plot.getHeight();if(!started){p.startNewSubPath(x,y);started=true;}else p.lineTo(x,y);}stroke(g,p,labelColour,1.5f);}
    else {constexpr size_t columns=604;for(int kind=0;kind<2;++kind){std::array<float,columns> low{},high{};std::array<bool,columns> seen{};for(int i=0;i<filled;++i){auto& v=history[size_t((cursor-filled+i+4096)%4096)];double age=now-v.time;if(age<0||age>1)continue;size_t x=size_t(juce::jlimit(0,603,int((1-age)*603)));float lo=kind?v.keyLo:v.outLo,hi=kind?v.keyHi:v.outHi;if(!std::isfinite(lo)||!std::isfinite(hi))continue;if(!seen[x]){low[x]=lo;high[x]=hi;seen[x]=true;}else{low[x]=juce::jmin(low[x],lo);high[x]=juce::jmax(high[x],hi);}}g.setColour(kind?labelColour.withAlpha(.8f):look.colour(0xff3794d4,0xffaeb3ba).withAlpha(.5f));for(size_t i=0;i<columns;++i)if(seen[i]){float x=plot.getX()+float(i)*plot.getWidth()/float(columns-1),a=plot.getCentreY()-juce::jlimit(-1.f,1.f,high[i])*plot.getHeight()*.5f,b=plot.getCentreY()-juce::jlimit(-1.f,1.f,low[i])*plot.getHeight()*.5f;g.drawLine(x,a,x,juce::jmax(a+.4f,b),.9f);}}}
}
void PhasePocketAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(look.colour(0xff060b12,0xffececeb));juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::scale(float(getWidth())/960));if(look.dark){g.setGradientFill(juce::ColourGradient(look.colour(0xff1d2b3e,0xffeeeeee),480,0,look.colour(0xff04080e,0xffeeeeee),480,800,true));g.fillRect(0,0,960,expanded?800:636);for(int i=0;i<7;++i){juce::Path p;p.startNewSubPath(340+float(i)*23,0);p.cubicTo(430+float(i)*20,70,470+float(i)*20,80,670+float(i)*25,95);stroke(g,p,look.colour(0xff52617e,0xff888888).withAlpha(.14f),.8f);}}
    text(g,"PHASE POCKET",{26,21,540,40},28,look.ink());g.setColour(look.colour(0xff263347,0xffc5c6c8));g.drawLine(24,78,936,78);
    graph(g,{24,94,674,230},true);graph(g,{24,344,674,230},false);panel(g,{714,94,222,480});
    text(g,"Sidechain",{69,592,170,26},13,look.muted());if(look.disabled)text(g,"BYPASSED",{720,592,208,26},13,look.muted(),juce::Justification::centredRight);
    if(expanded){panel(g,{24,637,912,143});text(g,"Sidechain filter",{44,648,178,28},16,look.ink());text(g,hz(sidechainRange.getMinValue()),{252,648,122,28},15,look.ink());text(g,hz(sidechainRange.getMaxValue()),{391,648,155,28},15,look.ink());text(g,"M/S",{48,741,52,25},13,look.muted());}
}
void PhasePocketAudioProcessorEditor::timerCallback(){if(resizeStamp>0&&juce::Time::getMillisecondCounterHiRes()-resizeStamp>400)saveSize();PocketTrace v;while(audioProcessor.popTrace(v)){if(!std::isfinite(v.time))continue;history[size_t(cursor)]=v;cursor=(cursor+1)%4096;filled=juce::jmin(filled+1,4096);}bool disabled=audioProcessor.parameters.getRawParameterValue("bypass")->load()>.5f||audioProcessor.displayBypass.load();if(disabled!=look.disabled){look.disabled=disabled;for(auto* c:getChildren())c->repaint();}repaint();}
