#pragma once

#include <JuceHeader.h>
#include "Utf8Text.h"
#include <cmath>

namespace SceneChrome
{
    struct StatusCardModel
    {
        juce::String badgeText;
        juce::Colour badgeColour = juce::Colour::fromRGB (90, 161, 255);

        juce::String secondaryText;
        juce::Colour secondaryColour = juce::Colour::fromRGBA (255, 255, 255, 28);

        juce::String headline;
        juce::String body;

        float progressValue = 0.0f;
        juce::Colour progressColour = juce::Colour::fromRGB (97, 218, 251);

        juce::String footerLeft;
        juce::String footerCentre;
        juce::String footerRight;
    };

    inline void drawChip (juce::Graphics& g,
                          juce::Rectangle<float> area,
                          const juce::String& text,
                          juce::Colour fill,
                          float fontSize = 13.0f)
    {
        g.setColour (fill);
        g.fillRoundedRectangle (area, area.getHeight() * 0.5f);
        g.setColour (juce::Colours::white.withAlpha (0.98f));
        g.setFont (juce::FontOptions (fontSize, juce::Font::bold));
        g.drawText (text, area.toNearestInt(), juce::Justification::centred);
    }

    inline void drawGlassCard (juce::Graphics& g, juce::Rectangle<float> area, float radius)
    {
        g.setColour (juce::Colours::black.withAlpha (0.24f));
        g.fillRoundedRectangle (area.translated (0.0f, 7.0f), radius);

        juce::ColourGradient fill (juce::Colour::fromRGBA (255, 255, 255, 44), area.getCentreX(), area.getY(),
                                   juce::Colour::fromRGBA (255, 255, 255, 26), area.getCentreX(), area.getBottom(), false);
        g.setGradientFill (fill);
        g.fillRoundedRectangle (area, radius);

        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 26));
        g.drawRoundedRectangle (area.reduced (0.5f), radius, 1.0f);
    }

    inline void drawInsetPanel (juce::Graphics& g, juce::Rectangle<float> area, float radius)
    {
        g.setColour (juce::Colour::fromRGBA (26, 35, 51, 118));
        g.fillRoundedRectangle (area, radius);

        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 20));
        g.drawRoundedRectangle (area.reduced (0.5f), radius, 1.0f);
    }

    inline void drawProgressBar (juce::Graphics& g, juce::Rectangle<float> area, float progress, juce::Colour fillColour)
    {
        progress = juce::jlimit (0.0f, 1.0f, progress);

        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.fillRoundedRectangle (area, 9.0f);

        auto filled = area;
        filled.setWidth (juce::jmax (area.getHeight(), area.getWidth() * progress));
        g.setColour (fillColour);
        g.fillRoundedRectangle (filled, 9.0f);

        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawRoundedRectangle (area.reduced (0.5f), 9.0f, 1.0f);
    }

    inline void drawTopHud (juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            const juce::String& title,
                            const juce::String& stageText,
                            int totalStars,
                            int lastStars)
    {
        auto headerArea = juce::Rectangle<float> (28.0f, 24.0f, bounds.getWidth() - 56.0f, 86.0f);
        auto titleArea = headerArea.removeFromTop (40.0f);

        g.setColour (juce::Colours::white.withAlpha (0.96f));
        g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
        g.drawText (title, titleArea.toNearestInt(), juce::Justification::centred);

        auto hudRow = headerArea.withHeight (30.0f);

        const float stageChipWidth = juce::jlimit (320.0f, 440.0f, 182.0f + stageText.length() * 4.9f);
        const float starChipWidth = 110.0f;
        const float latestChipWidth = (lastStars > 0 ? 98.0f : 0.0f);
        const float gap1 = 10.0f;
        const float gap2 = (lastStars > 0 ? 8.0f : 0.0f);
        const float totalHudWidth = stageChipWidth + gap1 + starChipWidth + gap2 + latestChipWidth;
        const float startX = (bounds.getWidth() - totalHudWidth) * 0.5f;

        auto stageChip = juce::Rectangle<float> (startX, hudRow.getY(), stageChipWidth, hudRow.getHeight());
        auto starChip  = juce::Rectangle<float> (stageChip.getRight() + gap1, hudRow.getY(), starChipWidth, hudRow.getHeight());

        drawChip (g,
                  stageChip,
                  stageText.isNotEmpty() ? stageText : juce::String (u8s("Guidé | Standard")),
                  juce::Colour::fromRGBA (255, 255, 255, 22),
                  12.5f);

        drawChip (g,
                  starChip,
                  u8s("Étoiles ") + juce::String (juce::jmax (0, totalStars)),
                  juce::Colour::fromRGBA (255, 205, 72, 188),
                  13.0f);

        if (lastStars > 0)
        {
            auto latestChip = juce::Rectangle<float> (starChip.getRight() + gap2, hudRow.getY(), latestChipWidth, hudRow.getHeight());
            drawChip (g,
                      latestChip,
                      "+" + juce::String (lastStars) + u8s(" gagnées"),
                      juce::Colour::fromRGBA (66, 212, 123, 176),
                      12.0f);
        }
    }

    inline void drawStatusCard (juce::Graphics& g, juce::Rectangle<float> area, const StatusCardModel& model)
    {
        drawGlassCard (g, area, 22.0f);

        auto inset = area.reduced (12.0f, 10.0f);
        drawInsetPanel (g, inset, 18.0f);

        auto content = inset.reduced (16.0f, 12.0f);
        auto badgeRow = content.removeFromTop (22.0f);

        if (model.secondaryText.isNotEmpty())
        {
            const float secondaryWidth = juce::jlimit (84.0f, 140.0f, 22.0f + model.secondaryText.length() * 6.0f);
            auto rightBadge = badgeRow.removeFromRight (secondaryWidth);
            drawChip (g, rightBadge, model.secondaryText, model.secondaryColour, 11.2f);
            badgeRow.removeFromRight (8.0f);
        }

        const float badgeWidth = juce::jlimit (92.0f, 160.0f, 24.0f + model.badgeText.length() * 6.7f);
        auto leftBadge = badgeRow.removeFromLeft (badgeWidth);
        drawChip (g, leftBadge, model.badgeText, model.badgeColour, 11.5f);

        content.removeFromTop (8.0f);

        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (21.0f, juce::Font::bold));
        auto headlineArea = content.removeFromTop (24.0f);
        g.drawText (model.headline, headlineArea.toNearestInt(), juce::Justification::centred);

        g.setColour (juce::Colours::white.withAlpha (0.90f));
        g.setFont (juce::FontOptions (12.4f));
        auto bodyArea = content.removeFromTop (32.0f);
        g.drawFittedText (model.body, bodyArea.toNearestInt(), juce::Justification::centred, 2);

        auto progressArea = content.removeFromTop (12.0f);
        drawProgressBar (g, progressArea, model.progressValue, model.progressColour);

        content.removeFromTop (8.0f);
        auto footer = content.removeFromTop (18.0f);
        auto col1 = footer.removeFromLeft (footer.getWidth() * 0.34f);
        auto col2 = footer.removeFromLeft (footer.getWidth() * 0.32f);
        auto col3 = footer;

        g.setColour (juce::Colours::white.withAlpha (0.88f));
        g.setFont (juce::FontOptions (11.8f, juce::Font::bold));
        g.drawText (model.footerLeft,   col1.toNearestInt(), juce::Justification::centredLeft);
        g.drawText (model.footerCentre, col2.toNearestInt(), juce::Justification::centred);
        g.drawText (model.footerRight,  col3.toNearestInt(), juce::Justification::centredRight);
    }

    inline void drawCountdownOverlay (juce::Graphics& g,
                                      juce::Rectangle<float> bounds,
                                      float countdownRemaining,
                                      juce::Colour accentColour,
                                      const juce::String& title,
                                      const juce::String& subtitle)
    {
        auto overlay = juce::Rectangle<float> (220.0f, 184.0f).withCentre (bounds.getCentre());
        drawGlassCard (g, overlay, 28.0f);

        auto inset = overlay.reduced (12.0f, 12.0f);
        drawInsetPanel (g, inset, 22.0f);

        auto content = inset.reduced (20.0f, 18.0f);
        const int countValue = juce::jmax (1, static_cast<int> (std::ceil (countdownRemaining)));

        auto numberArea = content.removeFromTop (96.0f);
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions (74.0f, juce::Font::bold));
        g.drawText (juce::String (countValue), numberArea.toNearestInt(), juce::Justification::centredBottom);

        auto labelArea = content.removeFromTop (24.0f);
        drawChip (g, labelArea.withTrimmedLeft (26.0f).withTrimmedRight (26.0f), title, accentColour, 12.0f);

        g.setColour (juce::Colours::white.withAlpha (0.88f));
        g.setFont (juce::FontOptions (12.0f));
        g.drawFittedText (subtitle, content.toNearestInt(), juce::Justification::centredTop, 2);
    }
}
