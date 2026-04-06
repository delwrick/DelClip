#pragma once

#include <JuceHeader.h>

class DelClipAudioProcessor : public juce::AudioProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
public:
    DelClipAudioProcessor();
    ~DelClipAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi()  const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateParameters();

    // Tone — one-pole low-pass per channel
    juce::dsp::StateVariableTPTFilter<float> toneFilter;

    // Cached parameter values (updated on audio thread via atomic reads)
    std::atomic<float> driveGain  { 1.0f };
    std::atomic<float> outputGain { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelClipAudioProcessor)
};
