#include "PluginEditor.h"

namespace
{
juce::Colour background() { return juce::Colour(0xff050a12); }
juce::Colour primaryText() { return juce::Colour(0xffdbe5ff); }
juce::Colour secondaryText() { return juce::Colour(0xff8e9bb8); }
void drawLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
               float size, juce::Colour colour, int justification = juce::Justification::centredLeft)
{
    g.setColour(colour); g.setFont(juce::Font(juce::FontOptions(size))); g.drawText(text, area, justification);
}
}

ModernDial::ModernDial(juce::String newTitle, juce::String newSubtitle, juce::String newUnit,
                       juce::Colour newAccent, bool infinity)
    : title(std::move(newTitle)), subtitle(std::move(newSubtitle)), unit(std::move(newUnit)),
      accent(newAccent), infinityAtMaximum(infinity)
{
    setSliderStyle(juce::Slider::RotaryVerticalDrag);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setWantsKeyboardFocus(true);
}

void ModernDial::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float scale = juce::jmin(bounds.getWidth() / 180.0f, bounds.getHeight() / 220.0f);
    const float radius = juce::jmin(bounds.getWidth() * 0.39f, bounds.getHeight() * 0.32f);
    const auto centre = juce::Point<float>(bounds.getCentreX(), bounds.getY() + radius + 16.0f * scale);
    for (int layer = 5; layer >= 1; --layer)
    {
        const float spread = static_cast<float>(layer) * 3.2f * scale;
        g.setColour(juce::Colours::black.withAlpha(0.055f * static_cast<float>(6 - layer)));
        g.fillEllipse(centre.x - radius - spread, centre.y - radius + 8.0f * scale - spread * 0.25f,
                      2.0f * (radius + spread), 2.0f * (radius + spread));
    }
    juce::ColourGradient face(juce::Colour(0xff22344d), centre.x - radius, centre.y - radius,
                              juce::Colour(0xff050a11), centre.x + radius, centre.y + radius, false);
    face.addColour(0.38, juce::Colour(0xff14243a)); face.addColour(0.72, juce::Colour(0xff0a1421));
    g.setGradientFill(face); g.fillEllipse(centre.x - radius, centre.y - radius, 2.0f * radius, 2.0f * radius);
    g.setColour(juce::Colour(0xff304866)); g.drawEllipse(centre.x - radius, centre.y - radius, 2.0f * radius, 2.0f * radius, 1.3f * scale);
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    juce::Path rimHighlight;
    rimHighlight.addArc(centre.x - radius + 2.0f, centre.y - radius + 2.0f,
                        2.0f * radius - 4.0f, 2.0f * radius - 4.0f,
                        juce::MathConstants<float>::pi * 1.12f,
                        juce::MathConstants<float>::pi * 1.86f, true);
    g.strokePath(rimHighlight, juce::PathStrokeType(1.0f * scale));
    const float start = juce::MathConstants<float>::pi * 1.25f;
    const float end = juce::MathConstants<float>::pi * 2.75f;
    const float proportion = static_cast<float>(valueToProportionOfLength(getValue()));
    juce::Path track, arc;
    track.addCentredArc(centre.x, centre.y, radius + 8.0f * scale, radius + 8.0f * scale, 0.0f, start, end, true);
    g.setColour(juce::Colour(0xff111a27));
    g.strokePath(track, juce::PathStrokeType(8.0f * scale, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    if (proportion > 0.0f)
    {
        arc.addCentredArc(centre.x, centre.y, radius + 8.0f * scale, radius + 8.0f * scale,
                          0.0f, start, start + (end - start) * proportion, true);
        g.setColour(accent.withAlpha(0.14f));
        g.strokePath(arc, juce::PathStrokeType(16.0f * scale, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent.withAlpha(0.25f));
        g.strokePath(arc, juce::PathStrokeType(11.0f * scale, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::ColourGradient arcGradient(accent.brighter(0.55f), centre.x - radius, centre.y - radius,
                                         accent.darker(0.18f), centre.x + radius, centre.y + radius, false);
        arcGradient.addColour(0.5, accent); g.setGradientFill(arcGradient);
        g.strokePath(arc, juce::PathStrokeType(6.5f * scale, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    const float angle = start + (end - start) * proportion;
    const auto thumb = juce::Point<float>(centre.x + (radius + 8.0f * scale) * std::cos(angle),
                                          centre.y + (radius + 8.0f * scale) * std::sin(angle));
    g.setColour(accent.withAlpha(0.2f)); g.fillEllipse(thumb.x - 12.0f * scale, thumb.y - 12.0f * scale, 24.0f * scale, 24.0f * scale);
    g.setColour(juce::Colour(0xffeef5ff)); g.fillEllipse(thumb.x - 5.5f * scale, thumb.y - 5.5f * scale, 11.0f * scale, 11.0f * scale);
    g.setColour(accent.brighter(0.35f)); g.drawEllipse(thumb.x - 5.5f * scale, thumb.y - 5.5f * scale, 11.0f * scale, 11.0f * scale, 1.4f * scale);
    drawLabel(g, title, { centre.x - radius, centre.y - radius * 0.55f, 2.0f * radius, 24.0f * scale }, 16.0f * scale, primaryText(), juce::Justification::centred);
    drawLabel(g, subtitle, { centre.x - radius, centre.y - radius * 0.25f, 2.0f * radius, 20.0f * scale }, 12.0f * scale, secondaryText(), juce::Justification::centred);
    const bool showsInfinity = infinityAtMaximum && proportion > 0.9995f;
    const juce::String value = showsInfinity ? juce::String::fromUTF8("∞") : juce::String(getValue(), getInterval() < 1.0 ? 1 : 0);
    drawLabel(g, value, { centre.x - radius, centre.y - 3.0f * scale, 2.0f * radius, 47.0f * scale }, 34.0f * scale, primaryText(), juce::Justification::centred);
    if (!showsInfinity) drawLabel(g, unit, { centre.x - radius, centre.y + 39.0f * scale, 2.0f * radius, 18.0f * scale }, 12.0f * scale, secondaryText(), juce::Justification::centred);
}

void RangeLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float, float minSliderPos, float maxSliderPos,
                                        juce::Slider::SliderStyle style, juce::Slider&)
{
    if (style != juce::Slider::TwoValueHorizontal) return;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const auto fullTrack = juce::Rectangle<float>(static_cast<float>(x), centreY - 3.5f, static_cast<float>(width), 7.0f);
    g.setColour(juce::Colours::black.withAlpha(0.55f)); g.fillRoundedRectangle(fullTrack.translated(0.0f, 4.0f), 4.0f);
    g.setColour(juce::Colour(0xff172438)); g.fillRoundedRectangle(fullTrack, 4.0f);
    const auto selected = juce::Rectangle<float>(minSliderPos, centreY - 3.5f, juce::jmax(1.0f, maxSliderPos - minSliderPos), 7.0f);
    g.setColour(juce::Colour(0xff2c8fff).withAlpha(0.18f)); g.fillRoundedRectangle(selected.expanded(3.0f), 6.0f);
    juce::ColourGradient band(juce::Colour(0xff387bff), minSliderPos, centreY, juce::Colour(0xff3ce5df), maxSliderPos, centreY, false);
    band.addColour(0.5, juce::Colour(0xff50b7ff)); g.setGradientFill(band); g.fillRoundedRectangle(selected, 4.0f);
    for (const float thumbX : { minSliderPos, maxSliderPos })
    {
        g.setColour(juce::Colour(0xff4aaeff).withAlpha(0.18f)); g.fillEllipse(thumbX - 13.0f, centreY - 13.0f, 26.0f, 26.0f);
        g.setColour(juce::Colour(0xffedf5ff)); g.fillEllipse(thumbX - 7.0f, centreY - 7.0f, 14.0f, 14.0f);
        g.setColour(juce::Colour(0xff628dff)); g.drawEllipse(thumbX - 7.0f, centreY - 7.0f, 14.0f, 14.0f, 2.0f);
    }
}

PhasePocketAudioProcessorEditor::PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setOpaque(true); setResizable(true, true); setResizeLimits(900, 672, 1800, 1344);
    getConstrainer()->setFixedAspectRatio(75.0 / 56.0);
    const std::array<juce::Component*, 10> components { &influence, &smoothing, &duration, &sustain, &sidechainRange, &midSide, &amplitude, &spectrum, &themeButton, &bypassButton };
    for (auto* component : components) addAndMakeVisible(component);
    sidechainRange.setSliderStyle(juce::Slider::TwoValueHorizontal);
    sidechainRange.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sidechainRange.setRange(20.0, 20000.0, 1.0); sidechainRange.setSkewFactorFromMidPoint(1000.0);
    sidechainRange.setLookAndFeel(&rangeLookAndFeel);
    sidechainRange.setMinAndMaxValues(audioProcessor.parameters.getRawParameterValue("scLow")->load(), audioProcessor.parameters.getRawParameterValue("scHigh")->load(), juce::dontSendNotification);
    sidechainRange.onDragStart = [this] { audioProcessor.parameters.getParameter("scLow")->beginChangeGesture(); audioProcessor.parameters.getParameter("scHigh")->beginChangeGesture(); };
    sidechainRange.onValueChange = [this] { setSidechainRangeParameter("scLow", static_cast<float>(sidechainRange.getMinValue())); setSidechainRangeParameter("scHigh", static_cast<float>(sidechainRange.getMaxValue())); };
    sidechainRange.onDragEnd = [this] { audioProcessor.parameters.getParameter("scLow")->endChangeGesture(); audioProcessor.parameters.getParameter("scHigh")->endChangeGesture(); };
    midSide.setSliderStyle(juce::Slider::LinearHorizontal); midSide.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 26);
    midSide.setColour(juce::Slider::trackColourId, juce::Colour(0xff5978ec)); midSide.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff172235));
    midSide.setColour(juce::Slider::thumbColourId, juce::Colour(0xffdce8ff)); midSide.setColour(juce::Slider::textBoxTextColourId, primaryText());
    midSide.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0b1420)); midSide.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff263449));
    influenceAttach = std::make_unique<SliderAttachment>(p.parameters, "amount", influence);
    smoothingAttach = std::make_unique<SliderAttachment>(p.parameters, "release", smoothing);
    durationAttach = std::make_unique<SliderAttachment>(p.parameters, "duration", duration);
    sustainAttach = std::make_unique<SliderAttachment>(p.parameters, "sustain", sustain);
    msAttach = std::make_unique<SliderAttachment>(p.parameters, "msBalance", midSide);
    bypassAttach = std::make_unique<ButtonAttachment>(p.parameters, "bypass", bypassButton);
    amplitude.onClick = [this] { setMode(1.0f); }; spectrum.onClick = [this] { setMode(0.0f); }; bypassButton.setClickingTogglesState(true);
    const std::array<juce::TextButton*, 4> buttons { &amplitude, &spectrum, &themeButton, &bypassButton };
    for (auto* button : buttons) { button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0d1724)); button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff405fd0)); button->setColour(juce::TextButton::textColourOffId, secondaryText()); button->setColour(juce::TextButton::textColourOnId, primaryText()); }
    influence.setTooltip("Processing depth"); smoothing.setTooltip("Movement inside an active event; it does not extend the event tail");
    duration.setTooltip("Maximum full sidechain duration. Maximum means infinity"); sustain.setTooltip("Amount of sidechain retained after Duration"); sidechainRange.setTooltip("Sidechain detector frequency range");
    int initial = audioProcessor.editorWidth.load(); if (initial < 900 || initial > 1800) initial = 1200;
    setSize(initial, juce::roundToInt(static_cast<double>(initial) * 56.0 / 75.0));
    audioProcessor.editorWidth.store(initial); audioProcessor.editorOpen.store(true); startTimerHz(30);
}

PhasePocketAudioProcessorEditor::~PhasePocketAudioProcessorEditor() { stopTimer(); sidechainRange.setLookAndFeel(nullptr); audioProcessor.editorOpen.store(false); }
void PhasePocketAudioProcessorEditor::setMode(float value) { auto* parameter = audioProcessor.parameters.getParameter("mode"); parameter->beginChangeGesture(); parameter->setValueNotifyingHost(value); parameter->endChangeGesture(); }
void PhasePocketAudioProcessorEditor::setSidechainRangeParameter(const char* id, float value) { auto* parameter = audioProcessor.parameters.getParameter(id); const float normalised = parameter->convertTo0to1(value); if (std::abs(parameter->getValue() - normalised) > 1.0e-6f) parameter->setValueNotifyingHost(normalised); }
juce::Rectangle<int> PhasePocketAudioProcessorEditor::scaled(float x, float y, float w, float h) const { const float scale = static_cast<float>(getWidth()) / 1200.0f; return { juce::roundToInt(x * scale), juce::roundToInt(y * scale), juce::roundToInt(w * scale), juce::roundToInt(h * scale) }; }
void PhasePocketAudioProcessorEditor::resized() { amplitude.setBounds(scaled(820,34,125,44)); spectrum.setBounds(scaled(945,34,125,44)); themeButton.setBounds(scaled(1080,34,45,44)); bypassButton.setBounds(scaled(1132,34,45,44)); influence.setBounds(scaled(800,130,185,245)); smoothing.setBounds(scaled(990,130,185,245)); duration.setBounds(scaled(800,390,185,245)); sustain.setBounds(scaled(990,390,185,245)); sidechainRange.setBounds(scaled(80,738,1040,54)); midSide.setBounds(scaled(145,808,300,34)); audioProcessor.editorWidth.store(getWidth()); }

void PhasePocketAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<float> area, float radius)
{
    for (int layer = 4; layer >= 1; --layer) { g.setColour(juce::Colours::black.withAlpha(0.055f * static_cast<float>(5 - layer))); g.fillRoundedRectangle(area.translated(0.0f, static_cast<float>(layer) * 3.0f).expanded(static_cast<float>(layer)), radius + static_cast<float>(layer)); }
    juce::ColourGradient gradient(juce::Colour(0xff162438), area.getX(), area.getY(), juce::Colour(0xff070d16), area.getRight(), area.getBottom(), false);
    gradient.addColour(0.32, juce::Colour(0xff101c2b)); gradient.addColour(0.72, juce::Colour(0xff09131f)); g.setGradientFill(gradient); g.fillRoundedRectangle(area, radius);
    g.setColour(juce::Colour(0xff2d4058)); g.drawRoundedRectangle(area, radius, 1.1f); g.setColour(juce::Colours::white.withAlpha(0.055f)); g.drawRoundedRectangle(area.reduced(1.2f), radius - 1.2f, 1.0f);
}

void PhasePocketAudioProcessorEditor::drawSpectrum(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(0xff03070d)); g.fillRoundedRectangle(area, 9.0f);
    for (int i=0;i<=4;++i) { g.setColour(juce::Colour(0xff243246).withAlpha(0.65f)); g.drawHorizontalLine(juce::roundToInt(area.getY()+static_cast<float>(i)*area.getHeight()/4.0f),area.getX(),area.getRight()); g.drawVerticalLine(juce::roundToInt(area.getX()+static_cast<float>(i)*area.getWidth()/4.0f),area.getY(),area.getBottom()); }
    const auto gainToY=[&area](float gain){return area.getY()+area.getHeight()*(1.0f-std::pow(juce::jlimit(0.0f,1.0f,gain),0.25f));};
    for(int channel=1;channel>=0;--channel){const auto& values=channel!=0?spectrumCurve.side:spectrumCurve.mid;juce::Path path,fill;for(std::size_t i=0;i<values.size();++i){const float x=area.getX()+area.getWidth()*static_cast<float>(i)/static_cast<float>(values.size()-1);const float y=gainToY(values[i]);if(i==0){path.startNewSubPath(x,y);fill.startNewSubPath(x,area.getBottom());fill.lineTo(x,y);}else{path.lineTo(x,y);fill.lineTo(x,y);}}fill.lineTo(area.getRight(),area.getBottom());fill.closeSubPath();const auto colour=channel!=0?juce::Colour(0xff25d8d0):juce::Colour(0xff6384ff);g.setGradientFill(juce::ColourGradient(colour.withAlpha(0.30f),0.0f,area.getY(),colour.withAlpha(0.0f),0.0f,area.getBottom(),false));g.fillPath(fill);g.setColour(colour.withAlpha(0.16f));g.strokePath(path,juce::PathStrokeType(6.0f));g.setColour(colour);g.strokePath(path,juce::PathStrokeType(1.7f));}
}

void PhasePocketAudioProcessorEditor::drawScope(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(0xff03070d)); g.fillRoundedRectangle(area,9.0f); g.setColour(juce::Colour(0xff26364c)); g.drawHorizontalLine(juce::roundToInt(area.getCentreY()),area.getX(),area.getRight()); if(filled==0)return;
    const double now=history[static_cast<std::size_t>((cursor+4095)%4096)].time;
    for(int kind=0;kind<2;++kind){const auto colour=kind!=0?juce::Colour(0xffdbe5ff):juce::Colour(0xff2383d9);if(kind==0){g.setColour(colour.withAlpha(0.18f));for(int i=0;i<filled;++i){const auto& value=history[static_cast<std::size_t>((cursor-filled+i+4096)%4096)];const double age=now-value.time;if(age<0.0||age>1.0)continue;const float x=area.getRight()-static_cast<float>(age)*area.getWidth();g.drawLine(x,area.getCentreY()-value.outHi*area.getHeight()*0.48f,x,area.getCentreY()-value.outLo*area.getHeight()*0.48f,3.0f);}}g.setColour(colour.withAlpha(kind!=0?0.95f:0.78f));for(int i=0;i<filled;++i){const auto& value=history[static_cast<std::size_t>((cursor-filled+i+4096)%4096)];const double age=now-value.time;if(age<0.0||age>1.0)continue;const float x=area.getRight()-static_cast<float>(age)*area.getWidth();const float low=kind!=0?value.keyLo:value.outLo;const float high=kind!=0?value.keyHi:value.outHi;g.drawLine(x,area.getCentreY()-juce::jlimit(-1.0f,1.0f,high)*area.getHeight()*0.48f,x,area.getCentreY()-juce::jlimit(-1.0f,1.0f,low)*area.getHeight()*0.48f,kind!=0?1.2f:0.8f);}}
}

void PhasePocketAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background()); const float scale=static_cast<float>(getWidth())/1200.0f; juce::Graphics::ScopedSaveState save(g); g.addTransform(juce::AffineTransform::scale(scale));
    juce::ColourGradient vertical(juce::Colour(0xff121f32),600.0f,0.0f,juce::Colour(0xff03070d),600.0f,896.0f,false);vertical.addColour(0.28,juce::Colour(0xff08111d));vertical.addColour(0.68,juce::Colour(0xff050a12));g.setGradientFill(vertical);g.fillRect(0.0f,0.0f,1200.0f,896.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff28446c).withAlpha(0.26f),560.0f,20.0f,juce::Colours::transparentBlack,560.0f,470.0f,true));g.fillEllipse(180.0f,-270.0f,820.0f,650.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff133a4a).withAlpha(0.12f),1080.0f,380.0f,juce::Colours::transparentBlack,1080.0f,780.0f,true));g.fillEllipse(730.0f,80.0f,700.0f,700.0f);
    g.setColour(juce::Colours::black.withAlpha(0.18f));g.fillRect(0.0f,100.0f,1200.0f,40.0f);g.fillRect(0.0f,645.0f,1200.0f,791.0f-727.0f);
    for(int i=0;i<10;++i){juce::Path contour;contour.startNewSubPath(330.0f+static_cast<float>(i)*20.0f,-5.0f);contour.cubicTo(440.0f+static_cast<float>(i)*20.0f,60.0f,430.0f+static_cast<float>(i)*29.0f,125.0f,620.0f+static_cast<float>(i)*34.0f,168.0f);g.setColour(juce::Colour(0xff496487).withAlpha(0.12f+static_cast<float>(i%3)*0.025f));g.strokePath(contour,juce::PathStrokeType(1.0f));}
    drawLabel(g,"Phase Pocket",{115.0f,24.0f,360.0f,48.0f},38.0f,primaryText());drawLabel(g,"R  A  I  N  L  I  N  E",{118.0f,72.0f,300.0f,20.0f},13.0f,secondaryText());
    for(int i=0;i<5;++i){const float x=42.0f+static_cast<float>(i)*11.0f;const float barHeight=20.0f+static_cast<float>(2-std::abs(2-i))*13.0f;g.setColour(juce::Colour(0xff4f7cff).withAlpha(0.18f));g.fillRoundedRectangle(x-5.0f,45.0f-barHeight*0.5f-5.0f,15.0f,barHeight+10.0f,7.5f);juce::ColourGradient bar(juce::Colour(0xff756dff),x,25.0f,juce::Colour(0xff27d8dc),x,76.0f,false);g.setGradientFill(bar);g.fillRoundedRectangle(x,45.0f-barHeight*0.5f,5.0f,barHeight,2.5f);}
    drawPanel(g,{25.0f,120.0f,750.0f,540.0f});drawPanel(g,{790.0f,120.0f,385.0f,540.0f});drawPanel(g,{25.0f,680.0f,1150.0f,185.0f});
    drawLabel(g,"Spectral reduction",{50.0f,142.0f,350.0f,30.0f},22.0f,primaryText());drawLabel(g,"M",{690.0f,143.0f,25.0f,28.0f},16.0f,juce::Colour(0xff6384ff),juce::Justification::centred);drawLabel(g,"S",{720.0f,143.0f,25.0f,28.0f},16.0f,juce::Colour(0xff25d8d0),juce::Justification::centred);
    drawSpectrum(g,{100.0f,185.0f,640.0f,190.0f});drawLabel(g,"0 dB",{47.0f,178.0f,52.0f,22.0f},14.0f,secondaryText());drawLabel(g,"-inf",{55.0f,352.0f,40.0f,22.0f},14.0f,secondaryText());drawLabel(g,"20",{92.0f,380.0f,45.0f,20.0f},13.0f,secondaryText());drawLabel(g,"20k",{705.0f,380.0f,40.0f,20.0f},13.0f,secondaryText(),juce::Justification::centredRight);
    drawLabel(g,"Oscilloscope",{50.0f,408.0f,350.0f,30.0f},22.0f,primaryText());drawScope(g,{50.0f,450.0f,690.0f,170.0f});drawLabel(g,"-1 s",{50.0f,625.0f,50.0f,20.0f},13.0f,secondaryText());drawLabel(g,"Now",{690.0f,625.0f,50.0f,20.0f},13.0f,secondaryText(),juce::Justification::centredRight);
    drawLabel(g,"Sidechain filter",{50.0f,696.0f,190.0f,28.0f},19.0f,primaryText());drawLabel(g,"Detector frequency range",{50.0f,720.0f,220.0f,22.0f},12.0f,secondaryText());
    const auto formatFrequency=[](double value){return value>=1000.0?juce::String(value/1000.0,1)+" kHz":juce::String(juce::roundToInt(value))+" Hz";};
    drawLabel(g,formatFrequency(sidechainRange.getMinValue()),{78.0f,718.0f,120.0f,24.0f},13.0f,juce::Colour(0xff6e93ff));drawLabel(g,formatFrequency(sidechainRange.getMaxValue()),{1000.0f,718.0f,120.0f,24.0f},13.0f,juce::Colour(0xff45ddd7),juce::Justification::centredRight);
    drawLabel(g,"M / S focus",{50.0f,808.0f,95.0f,28.0f},14.0f,secondaryText());drawLabel(g,"v0.6.0",{1085.0f,825.0f,65.0f,24.0f},14.0f,secondaryText(),juce::Justification::centredRight);
    if(bypassLook){g.setColour(juce::Colours::black.withAlpha(0.42f));g.fillRect(0.0f,105.0f,1200.0f,791.0f);}
}

void PhasePocketAudioProcessorEditor::timerCallback()
{
    PocketTrace trace;while(audioProcessor.popTrace(trace)){if(!std::isfinite(trace.time))continue;history[static_cast<std::size_t>(cursor)]=trace;cursor=(cursor+1)%4096;filled=juce::jmin(filled+1,4096);}SpectrumTrace curve;while(audioProcessor.popSpectrum(curve))spectrumCurve=curve;
    if(!sidechainRange.isMouseButtonDown()){const float low=audioProcessor.parameters.getRawParameterValue("scLow")->load();const float high=audioProcessor.parameters.getRawParameterValue("scHigh")->load();if(std::abs(static_cast<float>(sidechainRange.getMinValue())-low)>0.5f||std::abs(static_cast<float>(sidechainRange.getMaxValue())-high)>0.5f)sidechainRange.setMinAndMaxValues(low,high,juce::dontSendNotification);}
    const bool amplitudeMode=audioProcessor.parameters.getRawParameterValue("mode")->load()>0.5f;amplitude.setToggleState(amplitudeMode,juce::dontSendNotification);spectrum.setToggleState(!amplitudeMode,juce::dontSendNotification);bypassLook=audioProcessor.parameters.getRawParameterValue("bypass")->load()>0.5f||audioProcessor.displayBypass.load();repaint();
}
