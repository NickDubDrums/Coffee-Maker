#include "PluginEditor.h"
#include <BinaryData.h>

//==============================================================================
// Colour palette
const juce::Colour CoffeeMakerEditor::colBackground = juce::Colour (0xff1A0E08);
const juce::Colour CoffeeMakerEditor::colPanel       = juce::Colour (0xff2C1810);
const juce::Colour CoffeeMakerEditor::colText        = juce::Colour (0xffF5E6D3);
const juce::Colour CoffeeMakerEditor::colAccent      = juce::Colour (0xffB87333);
const juce::Colour CoffeeMakerEditor::colVuGreen     = juce::Colour (0xff4CAF50);
const juce::Colour CoffeeMakerEditor::colVuYellow    = juce::Colour (0xffFFEB3B);
const juce::Colour CoffeeMakerEditor::colVuRed       = juce::Colour (0xffE53935);

//==============================================================================
CoffeeMakerEditor::CoffeeMakerEditor (CoffeeMakerProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setSize (pluginWidth, pluginHeight);
    setResizable (false, false);
    backgroundImage = juce::ImageCache::getFromMemory (
        BinaryData::BG_png, BinaryData::BG_pngSize);
    startTimerHz (30);
}

CoffeeMakerEditor::~CoffeeMakerEditor()
{
    stopTimer();
}

//==============================================================================
void CoffeeMakerEditor::timerCallback()
{
    currentLevelDb[0] = audioProcessor.outputLevelDb[0].load (std::memory_order_relaxed);
    currentLevelDb[1] = audioProcessor.outputLevelDb[1].load (std::memory_order_relaxed);
    repaint();
}

//==============================================================================
void CoffeeMakerEditor::paint (juce::Graphics& g)
{
    drawBackground (g);
    drawWordmark   (g);
    drawVuMeters   (g);
}

//==============================================================================
void CoffeeMakerEditor::drawBackground (juce::Graphics& g) const
{
    if (backgroundImage.isValid())
        g.drawImage (backgroundImage,
                     getLocalBounds().toFloat(),
                     juce::RectanglePlacement::stretchToFit);
    else
        g.fillAll (colBackground);
}

void CoffeeMakerEditor::drawWordmark (juce::Graphics& g) const
{
    // Wordmark "COFFEE MAKER"
    g.setColour (colText);
    g.setFont (juce::Font (juce::FontOptions()
                                .withHeight (32.0f)
                                .withStyle ("Bold")));
    g.drawText ("COFFEE MAKER",
                0, 12, pluginWidth, 38,
                juce::Justification::centred, false);

    // Tagline
    g.setColour (colAccent);
    g.setFont (juce::Font (juce::FontOptions().withHeight (11.0f)));
    g.drawText ("Professional Mastering. Guaranteed Louder.",
                0, 52, pluginWidth, 18,
                juce::Justification::centred, false);

    // Decorative separator line
    g.setColour (colAccent.withAlpha (0.4f));
    g.drawHorizontalLine (74, 20.0f, pluginWidth - 20.0f);
}

void CoffeeMakerEditor::drawVuMeters (juce::Graphics& g) const
{
    // ── Two VU bars, centred, with dB scale labels on the right ─────────────
    const int barWidth   = 28;
    const int barHeight  = 160;
    const int gap        = 12;   // gap between L and R bars
    const int totalW     = barWidth * 2 + gap;
    const int barX_L     = (pluginWidth / 2) - (totalW / 2);
    const int barX_R     = barX_L + barWidth + gap;
    const int barY       = 100;

    // Channel labels
    g.setColour (colText.withAlpha (0.7f));
    g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));
    g.drawText ("L", barX_L, barY + barHeight + 4, barWidth, 14,
                juce::Justification::centred, false);
    g.drawText ("R", barX_R, barY + barHeight + 4, barWidth, 14,
                juce::Justification::centred, false);

    // Draw the two bars
    const juce::Rectangle<int> boundsL (barX_L, barY, barWidth, barHeight);
    const juce::Rectangle<int> boundsR (barX_R, barY, barWidth, barHeight);
    drawSingleVuBar (g, boundsL, currentLevelDb[0]);
    drawSingleVuBar (g, boundsR, currentLevelDb[1]);

    // ── dB scale labels to the right of the right bar ────────────────────────
    // The scale shows DISPLAY values (biased), labelled with mnemonic dB strings.
    // Positions are calculated from normalised (0=bottom, 1=top) display range.
    // Biased scale: displayed 0 dBFS = signal -12 dBFS, so labels are shifted.
    const int scaleX = barX_R + barWidth + 6;

    struct ScaleMark { float normPos; const char* label; };
    // These are the visual positions on the bar (0=bottom, 1=top) and what
    // the label says. We show signal dBFS values that map to those positions.
    static const ScaleMark marks[] = {
        { 1.00f, "0"  },
        { 0.83f, "-3" },
        { 0.67f, "-6" },
        { 0.50f, "-9" },
        { 0.33f, "-12"},
        { 0.17f, "-18"},
        { 0.00f, "-inf" }
    };

    g.setFont (juce::Font (juce::FontOptions().withHeight (9.0f)));
    g.setColour (colText.withAlpha (0.55f));
    for (auto& m : marks)
    {
        const int y = barY + barHeight - static_cast<int>(m.normPos * barHeight) - 5;
        g.drawText (m.label, scaleX, y, 22, 10,
                    juce::Justification::left, false);
        // Tick mark
        g.setColour (colText.withAlpha (0.2f));
        g.drawHorizontalLine (y + 5,
                              static_cast<float>(barX_R + barWidth + 2),
                              static_cast<float>(scaleX + 3));
        g.setColour (colText.withAlpha (0.55f));
    }
}

void CoffeeMakerEditor::drawSingleVuBar (juce::Graphics& g,
                                          juce::Rectangle<int> bounds,
                                          float levelDb) const
{
    const float norm = levelToNormalized (levelDb);
    const int   h    = bounds.getHeight();
    const int   w    = bounds.getWidth();
    const int   x    = bounds.getX();
    const int   y    = bounds.getY();

    // ── Background track ─────────────────────────────────────────────────────
    g.setColour (colBackground.brighter (0.05f));
    g.fillRect (bounds);
    g.setColour (colAccent.withAlpha (0.15f));
    g.drawRect (bounds, 1);

    // ── Filled level bar with gradient green→yellow→red ──────────────────────
    const int filledH = static_cast<int> (norm * h);
    if (filledH > 0)
    {
        juce::Rectangle<int> fillRect (x, y + h - filledH, w, filledH);

        juce::ColourGradient grad (colVuRed,    static_cast<float>(x), static_cast<float>(y),
                                   colVuGreen,  static_cast<float>(x), static_cast<float>(y + h),
                                   false);
        // Mid-point yellow at ~75% height
        grad.addColour (0.25, colVuYellow);

        g.setGradientFill (grad);
        g.fillRect (fillRect);
    }

    // ── LED segmentation: horizontal lines ───────────────────────────────────
    const int numSegments = 20;
    g.setColour (colBackground.withAlpha (0.55f));
    for (int seg = 1; seg < numSegments; ++seg)
    {
        const int lineY = y + (h * seg / numSegments);
        g.drawHorizontalLine (lineY, static_cast<float>(x), static_cast<float>(x + w));
    }

    // ── Peak hold dot (top of active bar) ────────────────────────────────────
    if (filledH > 3)
    {
        const int peakY = y + h - filledH;
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.fillRect (x + 2, peakY, w - 4, 2);
    }
}

//==============================================================================
float CoffeeMakerEditor::levelToNormalized (float levelDb) noexcept
{
    // Apply visual bias: add meterBiasDb so that moderate signals appear loud
    const float biased = levelDb + meterBiasDb;

    // Map from [-30, 0] dBFS (biased) → [0, 1] display range
    const float minDb = -30.0f;
    const float maxDb =   0.0f;
    return juce::jlimit (0.0f, 1.0f, (biased - minDb) / (maxDb - minDb));
}
