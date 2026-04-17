#pragma once

#include <JuceHeader.h>
#include "Utf8Text.h"
#include "AudioAnalyzer.h"
#include "GameSession.h"
#include "SettingsPanel.h"
#include "TreeScene.h"
#include "WindmillScene.h"
#include "ButterflyScene.h"

class MainComponent final : public juce::AudioAppComponent,
                            private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    class GameSelectionOverlay final : public juce::Component
    {
    public:
        void setCardBounds (juce::Rectangle<int> newCardBounds)
        {
            cardBounds = newCardBounds;
            repaint();
        }

        juce::Rectangle<int> getCardBounds() const noexcept { return cardBounds; }

        void paint (juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour (juce::Colour::fromRGBA (7, 14, 27, 112));
            g.fillRoundedRectangle (bounds, 24.0f);

            auto dimBand = bounds.reduced (18.0f, 22.0f);
            g.setColour (juce::Colour::fromRGBA (5, 12, 24, 82));
            g.fillRoundedRectangle (dimBand, 22.0f);

            auto card = cardBounds.toFloat();
            g.setColour (juce::Colours::black.withAlpha (0.26f));
            g.fillRoundedRectangle (card.translated (0.0f, 8.0f), 30.0f);

            juce::ColourGradient fill (juce::Colour::fromRGBA (16, 28, 48, 238), card.getCentreX(), card.getY(),
                                       juce::Colour::fromRGBA (26, 44, 74, 230), card.getCentreX(), card.getBottom(), false);
            fill.addColour (0.55, juce::Colour::fromRGBA (22, 38, 63, 234));
            g.setGradientFill (fill);
            g.fillRoundedRectangle (card, 30.0f);

            g.setColour (juce::Colour::fromRGBA (255, 255, 255, 36));
            g.drawRoundedRectangle (card.reduced (0.5f), 30.0f, 1.2f);

            auto sheen = card.reduced (18.0f, 18.0f).removeFromTop (46.0f);
            juce::ColourGradient glow (juce::Colour::fromRGBA (132, 205, 255, 28), sheen.getCentreX(), sheen.getY(),
                                       juce::Colour::fromRGBA (132, 205, 255, 0), sheen.getCentreX(), sheen.getBottom(), false);
            g.setGradientFill (glow);
            g.fillRoundedRectangle (sheen, 18.0f);
        }

    private:
        juce::Rectangle<int> cardBounds;
    };

    void timerCallback() override;
    void initialiseAudio();
    void startSession();
    void resetSession();
    void refreshUi();
    void updateSceneVisibility();
    void updateGameSelectionOverlay();

    GameSelectionOverlay gameSelectionOverlay;
    juce::Label gameSelectionTitleLabel;
    juce::Label gameSelectionSubtitleLabel;
    juce::TextButton treeGameButton { "Guirlande du sapin" };
    juce::TextButton windmillGameButton { u8s("Moulin à vent") };
    juce::TextButton butterflyGameButton { "Vol du papillon" };

    AudioAnalyzer analyzer;
    GameSession session;
    TreeScene treeScene;
    WindmillScene windmillScene;
    ButterflyScene butterflyScene;
    SettingsPanel settingsPanel;

    bool audioReady = false;
    bool permissionDenied = false;
    double lastUiTickMs = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
