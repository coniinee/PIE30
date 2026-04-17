#pragma once

#include <JuceHeader.h>
#include <atomic>

class AudioAnalyzer
{
public:
    void prepare (double sampleRate, int maxSamplesPerBlock);
    void reset();

    void startCalibration (double seconds = 1.5);
    void setSensitivityOffsetDb (float offsetDb);

    void processBlock (const juce::AudioBuffer<float>& buffer,
                       int startSample,
                       int numSamples,
                       int numInputChannels);

    float getRawDb() const noexcept               { return rawDb.load(); }
    float getSmoothedDb() const noexcept          { return smoothedDb.load(); }
    float getNoiseFloorDb() const noexcept        { return noiseFloorDb.load(); }
    float getOpenThresholdDb() const noexcept     { return openThresholdDb.load(); }
    float getCloseThresholdDb() const noexcept    { return closeThresholdDb.load(); }
    float getSensitivityOffsetDb() const noexcept { return sensitivityOffsetDb.load(); }
    float getIntensityNormalised() const noexcept { return intensityNormalised.load(); }
    float getPitchHz() const noexcept             { return pitchHz.load(); }
    float getPitchNormalised() const noexcept     { return pitchNormalised.load(); }

    bool isSoundDetected() const noexcept         { return soundDetected.load(); }
    bool isCalibrating() const noexcept           { return calibrating.load(); }
    float getCalibrationProgress() const noexcept { return calibrationProgress.load(); }

private:
    void recalculateThresholdsLocked() noexcept;
    void finishCalibrationLocked() noexcept;
    void resetPitchTrackingLocked() noexcept;
    void pushInputSamplesToPitchHistory (const juce::AudioBuffer<float>& buffer,
                                         int startSample,
                                         int numSamples) noexcept;
    float estimatePitchHzFromHistory() noexcept;
    static float pitchHzToNormalised (float hz) noexcept;

    juce::SpinLock stateLock;

    double currentSampleRate = 44100.0;
    int expectedBlockSize = 512;

    static constexpr float minDb = -100.0f;
    static constexpr float smoothAlpha = 0.88f;
    static constexpr float pitchSmoothAlpha = 0.72f;
    static constexpr float openMarginDb = 10.0f;
    static constexpr float closeMarginDb = 7.0f;
    static constexpr float intensitySpanDb = 22.0f;
    static constexpr float minPitchHz = 95.0f;
    static constexpr float maxPitchHz = 560.0f;
    static constexpr float gamePitchMinHz = 140.0f;
    static constexpr float gamePitchMaxHz = 520.0f;
    static constexpr float minPitchCorrelation = 0.54f;
    static constexpr float nearPeakCorrelationRatio = 0.92f;
    static constexpr float pitchGateMarginDb = 0.75f;
    static constexpr double startHoldSeconds = 0.14;
    static constexpr double stopHoldSeconds = 0.20;
    static constexpr int minStablePitchBlocks = 1;
    static constexpr int maxInvalidPitchHoldBlocks = 4;

    std::atomic<float> rawDb { minDb };
    std::atomic<float> smoothedDb { minDb };
    std::atomic<float> noiseFloorDb { -55.0f };
    std::atomic<float> openThresholdDb { -45.0f };
    std::atomic<float> closeThresholdDb { -48.0f };
    std::atomic<float> sensitivityOffsetDb { 0.0f };
    std::atomic<float> calibrationProgress { 0.0f };
    std::atomic<float> intensityNormalised { 0.0f };
    std::atomic<float> pitchHz { 0.0f };
    std::atomic<float> pitchNormalised { 0.0f };

    std::atomic<bool> soundDetected { false };
    std::atomic<bool> calibrating { false };

    double calibrationDurationSec = 1.5;
    double elapsedCalibrationSec = 0.0;
    double calibrationDbSum = 0.0;
    int calibrationBlockCount = 0;

    double aboveThresholdSec = 0.0;
    double belowThresholdSec = 0.0;

    juce::HeapBlock<float> pitchScratch;
    juce::HeapBlock<float> pitchHistory;
    int pitchScratchCapacity = 0;
    int pitchHistoryCapacity = 0;
    int pitchHistoryWritePos = 0;
    int pitchHistoryCount = 0;
    int pitchWindowSamples = 2048;

    float recentPitchCandidates[3] { 0.0f, 0.0f, 0.0f };
    int recentPitchCandidateCount = 0;
    int recentPitchCandidateIndex = 0;
    float lastRawCandidatePitchHz = 0.0f;
    int consistentPitchBlocks = 0;
    int invalidPitchBlocks = 0;
};
