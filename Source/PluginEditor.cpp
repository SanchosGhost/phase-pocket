#include "PluginEditor.h"

namespace Theme
{
const auto canvas = juce::Colour::fromRGB(25, 25, 25);
const auto surface = juce::Colour::fromRGB(32, 32, 32);
const auto raised = juce::Colour::fromRGB(56, 56, 54);
const auto text = juce::Colours::white;
const auto secondary = juce::Colour::fromFloatRGBA(1, 1, 1, 0.65f);
const auto border = juce::Colour::fromFloatRGBA(1, 1, 1, 0.16f);
const auto accent = juce::Colour::fromRGB(94, 159, 232);
const auto green = juce::Colour::fromRGB(114, 188, 143);
}

PhasePocketLookAndFeel::PhasePocketLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, Theme::text);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ToggleButton::textColourId, Theme::secondary);
    setColour(juce::ToggleButton::tickColourId, Theme::accent);
}

void PhasePocketLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float position, float start, float end, juce::Slider&)
{
    auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(10.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = start + position * (end - start);
    g.setColour(Theme::raised); g.fillEllipse(bounds);
    g.setColour(Theme::border); g.drawEllipse(bounds, 1.0f);
    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, start, angle, true);
    g.setColour(Theme::accent);
    g.strokePath(arc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path pointer;
    pointer.addRoundedRectangle(-1.5f, -radius + 8.0f, 3.0f, radius * 0.42f, 1.5f);
    g.setColour(Theme::text);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

PhasePocketAudioProcessorEditor::PhasePocketAudioProcessorEditor(PhasePocketAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel); setSize(720, 430);
    configureKnob(amount, amountLabel, "AMOUNT"); configureKnob(tolerance, toleranceLabel, "TOLERANCE");
    configureKnob(low, lowLabel, "LOW"); configureKnob(high, highLabel, "HIGH");
    configureKnob(maxReduction, maxReductionLabel, "MAX REDUCTION"); configureKnob(release, releaseLabel, "RELEASE");
    amount.setTextValueSuffix(" %"); tolerance.setTextValueSuffix(" dB"); low.setTextValueSuffix(" Hz");
    high.setTextValueSuffix(" Hz"); maxReduction.setTextValueSuffix(" dB"); release.setTextValueSuffix(" ms");
    addAndMakeVisible(phaseAware); phaseAware.setClickingTogglesState(true);
    addAndMakeVisible(reductionReadout); reductionReadout.setJustificationType(juce::Justification::centredRight);
    reductionReadout.setColour(juce::Label::textColourId, Theme::green);
    reductionReadout.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    amountAttachment = std::make_unique<SliderAttachment>(processor.parameters, "amount", amount);
    toleranceAttachment = std::make_unique<SliderAttachment>(processor.parameters, "tolerance", tolerance);
    lowAttachment = std::make_unique<SliderAttachment>(processor.parameters, "low", low);
    highAttachment = std::make_unique<SliderAttachment>(processor.parameters, "high", high);
    maxReductionAttachment = std::make_unique<SliderAttachment>(processor.parameters, "maxReduction", maxReduction);
    releaseAttachment = std::make_unique<SliderAttachment>(processor.parameters, "release", release);
    phaseAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "phaseAware", phaseAware);
    startTimerHz(20);
}

PhasePocketAudioProcessorEditor::~PhasePocketAudioProcessorEditor() { setLookAndFeel(nullptr); }

void PhasePocketAudioProcessorEditor::configureKnob(juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 88, 22); addAndMakeVisible(slider);
    label.setText(labelText, juce::dontSendNotification); label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, Theme::secondary); label.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    addAndMakeVisible(label);
}

void PhasePocketAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(Theme::canvas);
    auto header = getLocalBounds().removeFromTop(86).toFloat();
    g.setColour(Theme::surface); g.fillRect(header); g.setColour(Theme::border);
    g.drawLine(0.0f, header.getBottom(), (float) getWidth(), header.getBottom());
    g.setColour(Theme::text); g.setFont(juce::FontOptions(27.0f, juce::Font::bold));
    g.drawText("PHASE POCKET", 28, 18, 300, 32, juce::Justification::centredLeft);
    g.setColour(Theme::secondary); g.setFont(juce::FontOptions(13.0f));
    g.drawText("phase-aware spectral sidechain", 30, 50, 300, 20, juce::Justification::centredLeft);
    auto card = juce::Rectangle<float>(24.0f, 108.0f, (float) getWidth() - 48.0f, 244.0f);
    g.setColour(Theme::surface); g.fillRoundedRectangle(card, 10.0f);
    g.setColour(Theme::border); g.drawRoundedRectangle(card, 10.0f, 1.0f);
    g.setColour(Theme::secondary); g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("SIDECHAIN PRIORITY · MAIN INPUT IS CARVED ONLY INSIDE THE FOCUS BAND", 30, 400, getWidth() - 60, 18, juce::Justification::centred);
}

void PhasePocketAudioProcessorEditor::resized()
{
    reductionReadout.setBounds(getWidth() - 190, 25, 160, 30); phaseAware.setBounds(30, 365, 150, 36);
    const int left = 34, top = 126, cellWidth = 108, gap = 2;
    juce::Slider* sliders[] { &amount, &tolerance, &low, &high, &maxReduction, &release };
    juce::Label* labels[] { &amountLabel, &toleranceLabel, &lowLabel, &highLabel, &maxReductionLabel, &releaseLabel };
    for (int i = 0; i < 6; ++i) { const int x = left + i * (cellWidth + gap); labels[i]->setBounds(x, top, cellWidth, 22); sliders[i]->setBounds(x, top + 20, cellWidth, 168); }
}

void PhasePocketAudioProcessorEditor::timerCallback()
{
    reductionReadout.setText(juce::String(processor.getReductionDb(), 1) + " dB REDUCTION", juce::dontSendNotification);
}
