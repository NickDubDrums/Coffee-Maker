#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

//==============================================================================
class CoffeeMakerProcessor : public juce::AudioProcessor
{
public:
    CoffeeMakerProcessor();
    ~CoffeeMakerProcessor() override = default;

    //==========================================================================
    // AudioProcessor overrides
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==========================================================================
    const juce::String getName() const override { return "Coffee Maker"; }

    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==========================================================================
    int getNumPrograms() override    { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==========================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // Metering: post-limiter peak level per channel, in dBFS.
    // Written lock-free in processBlock, read by the editor timer at 30 fps.
    static constexpr int maxChannels = 2;
    std::atomic<float> outputLevelDb[maxChannels];

private:
    //==========================================================================
    // DSP chain
    juce::dsp::Gain<float>    inputGain;
    juce::dsp::Limiter<float> limiter;

    // Soft-clipper constants
    static constexpr float drive          = 2.5f;
    static constexpr float tanhDrive      = 0.9866f; // tanh(2.5), pre-computed
    static constexpr float gainDb         = 4.5f;
    static constexpr float limiterCeiling = -0.03f;   // dBFS

    // Inline sample-level soft clip: tanh(x * drive) / tanh(drive)
    static inline float softClip (float x) noexcept
    {
        return std::tanh (x * drive) / tanhDrive;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoffeeMakerProcessor)
};
