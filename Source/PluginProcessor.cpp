#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelClipAudioProcessor::DelClipAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    apvts.addParameterListener ("drive",  this);
    apvts.addParameterListener ("tone",   this);
    apvts.addParameterListener ("output", this);
}

DelClipAudioProcessor::~DelClipAudioProcessor()
{
    apvts.removeParameterListener ("drive",  this);
    apvts.removeParameterListener ("tone",   this);
    apvts.removeParameterListener ("output", this);
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout DelClipAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Drive: 0 – 100 %
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "drive", 1 },
        "Drive",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
        50.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    // Tone: 500 Hz – 20 kHz (skewed so mid-knob ≈ 3–4 kHz)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "tone", 1 },
        "Tone",
        juce::NormalisableRange<float> (500.0f, 20000.0f, 1.0f, 0.35f),
        18000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    // Output Level: -24 – +12 dB
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 },
        "Output Level",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}

//==============================================================================
void DelClipAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == "drive")
    {
        // Map 0–100 % → pre-gain 1×–20× before tanh
        driveGain = 1.0f + (newValue / 100.0f) * 19.0f;
    }
    else if (parameterID == "tone")
    {
        toneFilter.setCutoffFrequency (newValue);
    }
    else if (parameterID == "output")
    {
        outputGain = juce::Decibels::decibelsToGain (newValue);
    }
}

//==============================================================================
void DelClipAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = static_cast<juce::uint32> (getTotalNumOutputChannels());

    toneFilter.prepare (spec);
    toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    toneFilter.setResonance (0.707f);   // Butterworth Q — flat passband

    updateParameters();
}

void DelClipAudioProcessor::releaseResources()
{
    toneFilter.reset();
}

//==============================================================================
void DelClipAudioProcessor::updateParameters()
{
    const float driveRaw  = apvts.getRawParameterValue ("drive")->load();
    const float toneRaw   = apvts.getRawParameterValue ("tone")->load();
    const float outputRaw = apvts.getRawParameterValue ("output")->load();

    driveGain  = 1.0f + (driveRaw / 100.0f) * 19.0f;
    outputGain = juce::Decibels::decibelsToGain (outputRaw);
    toneFilter.setCutoffFrequency (toneRaw);
}

//==============================================================================
bool DelClipAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

//==============================================================================
void DelClipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const float drive  = driveGain.load();
    const float output = outputGain.load();

    // Normalisation factor: tanh(drive * 1) / 1  →  keeps unity-gain at 0 dB drive
    // We divide by tanh(drive) so that a sine at 0 dBFS in = 0 dBFS out before output trim.
    const float normFactor = 1.0f / std::tanh (drive);

    // ── Waveshaper (tanh soft-clipper) ───────────────────────────────────────
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = std::tanh (data[i] * drive) * normFactor;
    }

    // ── Tone (low-pass) ──────────────────────────────────────────────────────
    auto block   = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (block);
    toneFilter.process (context);

    // ── Output level ─────────────────────────────────────────────────────────
    buffer.applyGain (output);
}

//==============================================================================
juce::AudioProcessorEditor* DelClipAudioProcessor::createEditor()
{
    return new DelClipAudioProcessorEditor (*this);
}

bool DelClipAudioProcessor::hasEditor() const { return true; }

//==============================================================================
const juce::String DelClipAudioProcessor::getName() const { return JucePlugin_Name; }
bool DelClipAudioProcessor::acceptsMidi()  const { return false; }
bool DelClipAudioProcessor::producesMidi() const { return false; }
bool DelClipAudioProcessor::isMidiEffect() const { return false; }
double DelClipAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
int  DelClipAudioProcessor::getNumPrograms()                        { return 1; }
int  DelClipAudioProcessor::getCurrentProgram()                     { return 0; }
void DelClipAudioProcessor::setCurrentProgram (int)                 {}
const juce::String DelClipAudioProcessor::getProgramName (int)      { return {}; }
void DelClipAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void DelClipAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void DelClipAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelClipAudioProcessor();
}
