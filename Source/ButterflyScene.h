#pragma once

#include <JuceHeader.h>
#include "GameSession.h"

class ButterflyScene : public juce::Component
{
public:
    ButterflyScene() = default;

    void setSceneData (GameSession::State newState,
                       float newProgress,
                       float newTargetProgress,
                       bool newSoundDetected,
                       float newPitchNormalised,
                       float newPitchHz,
                       float newCelebrateTime,
                       float newCalibrationProgress,
                       float newCountdownRemaining,
                       int newTotalStars,
                       int newLastStars,
                       const juce::String& newStageText,
                       const juce::String& newStatusText);

    void paint (juce::Graphics& g) override;

private:
    GameSession::State state = GameSession::State::idle;
    float progress = 0.0f;
    float targetProgress = 0.6f;
    bool soundDetected = false;
    float pitchNormalised = 0.0f;
    float pitchHz = 0.0f;
    float celebrateTime = 0.0f;
    float calibrationProgress = 0.0f;
    float countdownRemaining = 0.0f;
    int totalStars = 0;
    int lastStars = 0;
    juce::String stageText;
    juce::String statusText;

    float wingPhase = 0.0f;
    float hoverOffset = 0.0f;
    double lastFrameMs = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButterflyScene)
};
