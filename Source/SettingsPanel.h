#pragma once

#include <JuceHeader.h>
#include "Utf8Text.h"
#include <functional>

class SettingsPanel : public juce::Component
{
public:
    SettingsPanel();

    std::function<void()> onStartClicked;
    std::function<void()> onResetClicked;
    std::function<void(float)> onSensitivityChanged;
    std::function<void(int)> onGameChanged;
    std::function<void(int)> onModeChanged;
    std::function<void(int)> onDifficultyChanged;
    std::function<void(bool)> onCountdownToggled;

    void updateReadouts (float rawDb,
                         float smoothDb,
                         float noiseFloorDb,
                         float openThresholdDb,
                         float closeThresholdDb,
                         float intensityNormalised,
                         float pitchHz,
                         float pitchNormalised,
                         bool soundDetected,
                         const juce::String& statusText,
                         const juce::String& stageText,
                         const juce::String& statsText,
                         int totalStars,
                         int lastStars,
                         const juce::String& primaryActionText,
                         const juce::String& controlLabel,
                         float controlNormalised);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::Label stageLabel;
    juce::Label statusLabel;
    juce::Label meterLabel;
    juce::Label thresholdLabel;
    juce::Label sensitivityLabel;
    juce::Label gameLabel;
    juce::Label modeLabel;
    juce::Label difficultyLabel;
    juce::Label statsLabel;

    juce::ComboBox gameBox;
    juce::ComboBox modeBox;
    juce::ComboBox difficultyBox;
    juce::ToggleButton countdownToggle { u8s("Compte à rebours de 3 s après la calibration") };
    juce::Slider sensitivitySlider;
    juce::TextButton startButton { u8s("Démarrer le programme") };
    juce::TextButton resetButton { u8s("Réinitialiser") };

    juce::Rectangle<int> headerCardArea;
    juce::Rectangle<int> controlCardArea;
    juce::Rectangle<int> audioCardArea;
    juce::Rectangle<int> metricsCardArea;
    juce::Rectangle<int> actionsCardArea;

    juce::Rectangle<int> meterArea;
    juce::Rectangle<int> chipArea;
    juce::Rectangle<int> buttonsArea;

    juce::String cachedStageText;
    juce::String cachedStatusText;
    juce::String cachedStatsText;
    juce::String cachedPrimaryActionText;
    juce::String cachedMeterText;
    juce::String cachedThresholdText;

    float rawDb = -100.0f;
    float smoothDb = -100.0f;
    float noiseFloorDb = -55.0f;
    float openThresholdDb = -45.0f;
    float closeThresholdDb = -48.0f;
    float intensityNormalised = 0.0f;
    float pitchHz = 0.0f;
    float pitchNormalised = 0.0f;
    float controlNormalised = 0.0f;
    juce::String controlRowLabel { u8s("Contrôle") };
    bool soundDetected = false;
    int totalStars = 0;
    int lastStars = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPanel)
};
