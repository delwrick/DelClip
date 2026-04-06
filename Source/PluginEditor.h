#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class DelClipLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DelClipLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;
};

//==============================================================================
class DelClipAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit DelClipAudioProcessorEditor (DelClipAudioProcessor&);
    ~DelClipAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    DelClipAudioProcessor& audioProcessor;
    DelClipLookAndFeel lookAndFeel;

    juce::Slider driveKnob, toneKnob, outputKnob;
    juce::Label  driveLabel, toneLabel, outputLabel;
    juce::Label  valueLabel[3];   // live value readouts

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> driveAttachment, toneAttachment, outputAttachment;

    void setupKnob (juce::Slider& knob, juce::Label& label,
                    const juce::String& text, const juce::String& suffix);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelClipAudioProcessorEditor)
};
