#pragma once

#include <JuceHeader.h>
#include "GameSession.h"
#include <vector>

class TreeScene : public juce::Component
{
public:
    TreeScene() = default;

    void setSceneData (GameSession::State newState,
                       float newProgress,
                       float newTargetProgress,
                       bool newSoundDetected,
                       float newCelebrateTime,
                       float newCalibrationProgress,
                       float newCountdownRemaining,
                       int newTotalStars,
                       int newLastStars,
                       const juce::String& newStageText,
                       const juce::String& newStatusText);

    void paint (juce::Graphics& g) override;

private:
    static std::vector<juce::Point<float>> makeBulbPositions (const juce::Rectangle<float>& treeArea, int bulbCount);
    static juce::Path makeGarlandPath (const juce::Rectangle<float>& treeArea);

    GameSession::State state = GameSession::State::idle;
    float progress = 0.0f;
    float targetProgress = 1.0f;
    bool soundDetected = false;
    float celebrateTime = 0.0f;
    float calibrationProgress = 0.0f;
    float countdownRemaining = 0.0f;
    int totalStars = 0;
    int lastStars = 0;
    juce::String stageText;
    juce::String statusText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeScene)
};
