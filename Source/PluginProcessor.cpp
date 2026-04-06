#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CoffeeMakerProcessor::CoffeeMakerProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    for (auto& level : outputLevelDb)
        level.store (-96.0f, std::memory_order_relaxed);
}

//==============================================================================
void CoffeeMakerProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = static_cast<juce::uint32> (getTotalNumOutputChannels());

    // Stage 1 — fixed gain
    inputGain.setGainDecibels (gainDb);
    inputGain.prepare (spec);

    // Stage 3 — hard limiter
    limiter.setThreshold (limiterCeiling);
    limiter.setRelease   (50.0f);   // ms
    limiter.prepare (spec);

    // Reset meters
    for (auto& level : outputLevelDb)
        level.store (-96.0f, std::memory_order_relaxed);
}

void CoffeeMakerProcessor::releaseResources()
{
    inputGain.reset();
    limiter.reset();
}

//==============================================================================
void CoffeeMakerProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Silence unused output buses
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    // ── Stage 1: Gain (+4.5 dB) ─────────────────────────────────────────────
    {
        juce::dsp::AudioBlock<float>          block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        inputGain.process (ctx);
    }

    // ── Stage 2: Soft clipper (sample-by-sample, zero allocation) ───────────
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = softClip (data[i]);
    }

    // ── Stage 3: Hard limiter ────────────────────────────────────────────────
    {
        juce::dsp::AudioBlock<float>          block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        limiter.process (ctx);
    }

    // ── Metering: peak per channel, post-limiter ─────────────────────────────
    for (int ch = 0; ch < juce::jmin (numChannels, maxChannels); ++ch)
    {
        const float peak   = buffer.getMagnitude (ch, 0, numSamples);
        const float peakDb = (peak > 0.0f)
                                ? juce::Decibels::gainToDecibels (peak)
                                : -96.0f;
        outputLevelDb[ch].store (peakDb, std::memory_order_relaxed);
    }

    // Fill missing channels (mono input in stereo slot) with the first channel
    if (numChannels == 1)
        outputLevelDb[1].store (outputLevelDb[0].load (std::memory_order_relaxed),
                                std::memory_order_relaxed);
}

//==============================================================================
juce::AudioProcessorEditor* CoffeeMakerProcessor::createEditor()
{
    return new CoffeeMakerEditor (*this);
}

//==============================================================================
void CoffeeMakerProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // No user parameters — only save a version tag so the host is happy.
    juce::XmlElement xml ("CoffeeMakerState");
    xml.setAttribute ("version", "1.0.0");
    copyXmlToBinary (xml, destData);
}

void CoffeeMakerProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Nothing to restore — plugin has no parameters.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CoffeeMakerProcessor();
}
