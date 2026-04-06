#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
class CoffeeMakerEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit CoffeeMakerEditor (CoffeeMakerProcessor&);
    ~CoffeeMakerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override {}   // fixed layout — nothing to do

private:
    //==========================================================================
    // Timer callback: read levels, trigger repaint at 30 fps
    void timerCallback() override;

    //==========================================================================
    // Drawing helpers (all const — no state mutation in paint path)
    void drawBackground     (juce::Graphics&) const;
    void drawWordmark       (juce::Graphics&) const;
    void drawVuMeters       (juce::Graphics&) const;
    void drawSingleVuBar    (juce::Graphics&, juce::Rectangle<int> bounds,
                             float levelDb) const;

    //==========================================================================
    // dBFS → 0..1 display position with +12 dB bias:
    // -18 dBFS on signal → full scale on display (always in the red)
    static float levelToNormalized (float levelDb) noexcept;

    //==========================================================================
    // Colour constants — espresso / coffee palette
    static const juce::Colour colBackground;   // #1A0E08
    static const juce::Colour colPanel;        // #2C1810
    static const juce::Colour colText;         // #F5E6D3
    static const juce::Colour colAccent;       // #B87333
    static const juce::Colour colVuGreen;      // #4CAF50
    static const juce::Colour colVuYellow;     // #FFEB3B
    static const juce::Colour colVuRed;        // #E53935

    //==========================================================================
    CoffeeMakerProcessor& audioProcessor;

    // Cached meter levels (updated in timerCallback, read in paint)
    float currentLevelDb[2] { -96.0f, -96.0f };

    juce::Image backgroundImage;

    static constexpr int pluginWidth  = 400;
    static constexpr int pluginHeight = 300;

    // VU meter display bias: signal + bias → visual level
    // +12 dB means -18 dBFS signal appears as 0 dBFS on display
    static constexpr float meterBiasDb = 12.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoffeeMakerEditor)
};
