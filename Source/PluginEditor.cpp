#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Colour palette — named Theme to avoid clash with juce::Colours
namespace Theme
{
    static const juce::Colour background  { 0xff1c1c1e };
    static const juce::Colour surface     { 0xff2c2c2e };
    static const juce::Colour accent      { 0xffff6b35 };
    static const juce::Colour knobBody    { 0xff3a3a3c };
    static const juce::Colour knobTrack   { 0xff48484a };
    static const juce::Colour labelText   { 0xffaeaeb2 };
    static const juce::Colour valueText   { 0xffe5e5ea };
}

//==============================================================================
DelClipLookAndFeel::DelClipLookAndFeel()
{
    setColour (juce::Slider::thumbColourId,               Theme::accent);
    setColour (juce::Slider::rotarySliderFillColourId,    Theme::accent);
    setColour (juce::Slider::rotarySliderOutlineColourId, Theme::knobTrack);
    setColour (juce::Label::textColourId,                 Theme::labelText);
}

void DelClipLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                            int x, int y, int width, int height,
                                            float sliderPos,
                                            float startAngle, float endAngle,
                                            juce::Slider& /*slider*/)
{
    const float radius  = (float) juce::jmin (width, height) * 0.5f - 6.0f;
    const float centreX = (float) x + (float) width  * 0.5f;
    const float centreY = (float) y + (float) height * 0.5f;
    const float rx      = centreX - radius;
    const float ry      = centreY - radius;
    const float rw      = radius * 2.0f;
    const float angle   = startAngle + sliderPos * (endAngle - startAngle);

    // Track arc
    {
        juce::Path arc;
        arc.addArc (rx, ry, rw, rw, startAngle, endAngle, true);
        g.setColour (Theme::knobTrack);
        g.strokePath (arc, juce::PathStrokeType (3.5f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Value arc
    {
        juce::Path arc;
        arc.addArc (rx, ry, rw, rw, startAngle, angle, true);
        g.setColour (Theme::accent);
        g.strokePath (arc, juce::PathStrokeType (3.5f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Knob body
    const float innerR = radius * 0.72f;
    g.setColour (Theme::knobBody);
    g.fillEllipse (centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f);

    g.setColour (juce::Colour (0xff555558));
    g.drawEllipse (centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);

    // Pointer
    juce::Path pointer;
    const float pointerLen = innerR * 0.55f;
    pointer.startNewSubPath (0.0f, -innerR * 0.85f);
    pointer.lineTo (0.0f, -(innerR * 0.85f - pointerLen));
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    g.setColour (Theme::accent);
    g.strokePath (pointer, juce::PathStrokeType (2.5f,
        juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
DelClipAudioProcessorEditor::DelClipAudioProcessorEditor (DelClipAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&lookAndFeel);

    setupKnob (driveKnob,  driveLabel,  "DRIVE",  "%");
    setupKnob (toneKnob,   toneLabel,   "TONE",   "Hz");
    setupKnob (outputKnob, outputLabel, "OUTPUT", "dB");

    driveAttachment  = std::make_unique<Attachment> (p.apvts, "drive",  driveKnob);
    toneAttachment   = std::make_unique<Attachment> (p.apvts, "tone",   toneKnob);
    outputAttachment = std::make_unique<Attachment> (p.apvts, "output", outputKnob);

    setSize (360, 220);
}

DelClipAudioProcessorEditor::~DelClipAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void DelClipAudioProcessorEditor::setupKnob (juce::Slider& knob, juce::Label& label,
                                              const juce::String& text,
                                              const juce::String& /*suffix*/)
{
    knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 18);
    knob.setTextBoxIsEditable (true);
    knob.setColour (juce::Slider::textBoxTextColourId,       Theme::valueText);
    knob.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    knob.setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
    addAndMakeVisible (knob);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, Theme::labelText);
    addAndMakeVisible (label);
}

//==============================================================================
void DelClipAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::background);

    const int barH = 36;
    g.setColour (Theme::surface);
    g.fillRect (0, 0, getWidth(), barH);

    g.setColour (Theme::accent);
    g.fillRect (0, barH - 2, getWidth(), 2);

    g.setColour (Theme::valueText);
    g.setFont (juce::Font (juce::FontOptions (17.0f).withStyle ("Bold")));
    g.drawText ("DelClip", 0, 0, getWidth(), barH, juce::Justification::centred);

    g.setColour (Theme::labelText);
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText ("saturation / clipper", 0, 0, getWidth() - 10, barH,
                juce::Justification::centredRight);
}

void DelClipAudioProcessorEditor::resized()
{
    const int topBar   = 36;
    const int knobSize = 110;
    const int labelH   = 16;
    const int totalW   = getWidth();
    const int knobY    = topBar + (getHeight() - topBar - knobSize - labelH) / 2;
    const int labelY   = knobY + knobSize;
    const int colWidth = totalW / 3;

    auto placeKnob = [&] (juce::Slider& k, juce::Label& l, int col)
    {
        const int cx = col * colWidth + colWidth / 2;
        k.setBounds (cx - knobSize / 2, knobY, knobSize, knobSize);
        l.setBounds (cx - 50, labelY, 100, labelH);
    };

    placeKnob (driveKnob,  driveLabel,  0);
    placeKnob (toneKnob,   toneLabel,   1);
    placeKnob (outputKnob, outputLabel, 2);
}
