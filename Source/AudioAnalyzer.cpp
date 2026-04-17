#include "AudioAnalyzer.h"

namespace
{
    float safeNormalisedCorrelation (double numerator, double energyA, double energyB) noexcept
    {
        const auto denom = std::sqrt (juce::jmax (1.0e-12, energyA * energyB));
        return static_cast<float> (numerator / denom);
    }

    float medianOfThree (float a, float b, float c) noexcept
    {
        if (a > b)
            std::swap (a, b);
        if (b > c)
            std::swap (b, c);
        if (a > b)
            std::swap (a, b);
        return b;
    }
}

void AudioAnalyzer::prepare (double sampleRate, int maxSamplesPerBlock)
{
    const juce::SpinLock::ScopedLockType lock (stateLock);

    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    expectedBlockSize = juce::jmax (64, maxSamplesPerBlock);
    pitchWindowSamples = juce::jlimit (1536, 4096, juce::jmax (2048, expectedBlockSize * 4));

    pitchScratchCapacity = pitchWindowSamples;
    pitchScratch.malloc (static_cast<size_t> (pitchScratchCapacity));

    pitchHistoryCapacity = pitchWindowSamples * 2;
    pitchHistory.malloc (static_cast<size_t> (pitchHistoryCapacity));
    juce::FloatVectorOperations::clear (pitchHistory.getData(), pitchHistoryCapacity);

    rawDb.store (minDb);
    smoothedDb.store (minDb);
    intensityNormalised.store (0.0f);
    pitchHz.store (0.0f);
    pitchNormalised.store (0.0f);
    soundDetected.store (false);
    calibrationProgress.store (0.0f);

    aboveThresholdSec = 0.0;
    belowThresholdSec = 0.0;

    resetPitchTrackingLocked();
}

void AudioAnalyzer::reset()
{
    const juce::SpinLock::ScopedLockType lock (stateLock);

    rawDb.store (minDb);
    smoothedDb.store (minDb);
    intensityNormalised.store (0.0f);
    pitchHz.store (0.0f);
    pitchNormalised.store (0.0f);
    soundDetected.store (false);
    calibrating.store (false);
    calibrationProgress.store (0.0f);

    elapsedCalibrationSec = 0.0;
    calibrationDbSum = 0.0;
    calibrationBlockCount = 0;
    aboveThresholdSec = 0.0;
    belowThresholdSec = 0.0;

    resetPitchTrackingLocked();
    recalculateThresholdsLocked();
}

void AudioAnalyzer::startCalibration (double seconds)
{
    const juce::SpinLock::ScopedLockType lock (stateLock);

    calibrationDurationSec = juce::jlimit (0.25, 10.0, seconds);
    elapsedCalibrationSec = 0.0;
    calibrationDbSum = 0.0;
    calibrationBlockCount = 0;
    aboveThresholdSec = 0.0;
    belowThresholdSec = 0.0;

    rawDb.store (minDb);
    smoothedDb.store (minDb);
    intensityNormalised.store (0.0f);
    pitchHz.store (0.0f);
    pitchNormalised.store (0.0f);
    soundDetected.store (false);
    calibrating.store (true);
    calibrationProgress.store (0.0f);

    resetPitchTrackingLocked();
}

void AudioAnalyzer::setSensitivityOffsetDb (float offsetDb)
{
    const juce::SpinLock::ScopedLockType lock (stateLock);

    sensitivityOffsetDb.store (juce::jlimit (-12.0f, 12.0f, offsetDb));
    recalculateThresholdsLocked();
}

void AudioAnalyzer::processBlock (const juce::AudioBuffer<float>& buffer,
                                  int startSample,
                                  int numSamples,
                                  int numInputChannels)
{
    if (numSamples <= 0 || numInputChannels <= 0 || buffer.getNumChannels() <= 0)
        return;

    const auto channelsToRead = juce::jmin (numInputChannels, buffer.getNumChannels());

    double sumSquares = 0.0;
    int totalSamples = 0;

    for (int ch = 0; ch < channelsToRead; ++ch)
    {
        const auto* samples = buffer.getReadPointer (ch, startSample);

        for (int i = 0; i < numSamples; ++i)
        {
            const auto s = samples[i];
            sumSquares += static_cast<double> (s * s);
            ++totalSamples;
        }
    }

    if (totalSamples == 0)
        return;

    pushInputSamplesToPitchHistory (buffer, startSample, numSamples);

    const auto rms = static_cast<float> (std::sqrt (sumSquares / static_cast<double> (totalSamples)));
    const auto currentDb = juce::Decibels::gainToDecibels (juce::jmax (rms, 1.0e-6f), minDb);
    const auto previousSmooth = smoothedDb.load();
    const auto nextSmooth = smoothAlpha * previousSmooth + (1.0f - smoothAlpha) * currentDb;

    rawDb.store (currentDb);
    smoothedDb.store (nextSmooth);

    const auto blockSeconds = static_cast<double> (numSamples) / juce::jmax (1.0, currentSampleRate);

    const juce::SpinLock::ScopedLockType lock (stateLock);

    if (calibrating.load())
    {
        elapsedCalibrationSec += blockSeconds;
        calibrationDbSum += nextSmooth;
        ++calibrationBlockCount;

        calibrationProgress.store (juce::jlimit (0.0f, 1.0f,
                                                 static_cast<float> (elapsedCalibrationSec / calibrationDurationSec)));
        intensityNormalised.store (0.0f);
        pitchHz.store (0.0f);
        pitchNormalised.store (0.0f);
        soundDetected.store (false);
        aboveThresholdSec = 0.0;
        belowThresholdSec = 0.0;
        resetPitchTrackingLocked();

        if (elapsedCalibrationSec >= calibrationDurationSec)
            finishCalibrationLocked();

        return;
    }

    const auto openThreshold = openThresholdDb.load();
    const auto closeThreshold = closeThresholdDb.load();
    auto detected = soundDetected.load();

    if (! detected)
    {
        const auto openGate = nextSmooth >= openThreshold && currentDb >= (openThreshold - 1.0f);

        if (openGate)
            aboveThresholdSec += blockSeconds;
        else
            aboveThresholdSec = 0.0;

        belowThresholdSec = 0.0;

        if (aboveThresholdSec >= startHoldSeconds)
        {
            detected = true;
            aboveThresholdSec = 0.0;
        }
    }
    else
    {
        if (nextSmooth <= closeThreshold)
            belowThresholdSec += blockSeconds;
        else
            belowThresholdSec = 0.0;

        aboveThresholdSec = 0.0;

        if (belowThresholdSec >= stopHoldSeconds)
        {
            detected = false;
            belowThresholdSec = 0.0;
        }
    }

    soundDetected.store (detected);

    const auto intensity = juce::jmap (nextSmooth,
                                       closeThreshold,
                                       openThreshold + intensitySpanDb,
                                       0.0f,
                                       1.0f);
    intensityNormalised.store (juce::jlimit (0.0f, 1.0f, intensity));

    const auto pitchGateOpen = detected && nextSmooth >= (openThreshold + pitchGateMarginDb);

    const auto candidatePitchHz = pitchGateOpen ? estimatePitchHzFromHistory()
                                                : 0.0f;

    if (pitchGateOpen && candidatePitchHz > 0.0f)
    {
        const auto jumpLooksReasonable = lastRawCandidatePitchHz <= 0.0f
                                         || (candidatePitchHz >= lastRawCandidatePitchHz * 0.80f
                                             && candidatePitchHz <= lastRawCandidatePitchHz * 1.25f);

        consistentPitchBlocks = jumpLooksReasonable ? (consistentPitchBlocks + 1) : 1;
        lastRawCandidatePitchHz = candidatePitchHz;
        invalidPitchBlocks = 0;

        if (consistentPitchBlocks >= minStablePitchBlocks)
        {
            recentPitchCandidates[recentPitchCandidateIndex] = candidatePitchHz;
            recentPitchCandidateIndex = (recentPitchCandidateIndex + 1) % 3;
            recentPitchCandidateCount = juce::jmin (3, recentPitchCandidateCount + 1);

            float filteredCandidate = candidatePitchHz;
            if (recentPitchCandidateCount == 2)
                filteredCandidate = 0.5f * (recentPitchCandidates[0] + recentPitchCandidates[1]);
            else if (recentPitchCandidateCount >= 3)
                filteredCandidate = medianOfThree (recentPitchCandidates[0],
                                                   recentPitchCandidates[1],
                                                   recentPitchCandidates[2]);

            const auto previousPitch = pitchHz.load();
            const auto smoothedPitch = previousPitch > 0.0f
                                         ? pitchSmoothAlpha * previousPitch + (1.0f - pitchSmoothAlpha) * filteredCandidate
                                         : filteredCandidate;

            pitchHz.store (smoothedPitch);
            pitchNormalised.store (pitchHzToNormalised (smoothedPitch));
        }
    }
    else
    {
        consistentPitchBlocks = 0;
        lastRawCandidatePitchHz = 0.0f;
        recentPitchCandidateCount = 0;
        recentPitchCandidateIndex = 0;

        if (! detected)
        {
            resetPitchTrackingLocked();
        }
        else
        {
            ++invalidPitchBlocks;
            if (invalidPitchBlocks > maxInvalidPitchHoldBlocks)
            {
                pitchHz.store (0.0f);
                pitchNormalised.store (0.0f);
            }
        }
    }
}

void AudioAnalyzer::pushInputSamplesToPitchHistory (const juce::AudioBuffer<float>& buffer,
                                                    int startSample,
                                                    int numSamples) noexcept
{
    if (pitchHistoryCapacity <= 0 || buffer.getNumChannels() <= 0)
        return;

    const auto* source = buffer.getReadPointer (0, startSample);

    for (int i = 0; i < numSamples; ++i)
    {
        pitchHistory[pitchHistoryWritePos] = source[i];
        pitchHistoryWritePos = (pitchHistoryWritePos + 1) % pitchHistoryCapacity;
        pitchHistoryCount = juce::jmin (pitchHistoryCapacity, pitchHistoryCount + 1);
    }
}

float AudioAnalyzer::estimatePitchHzFromHistory() noexcept
{

    if (pitchScratchCapacity < pitchWindowSamples || pitchHistoryCount < pitchWindowSamples)
        return 0.0f;

    const auto firstIndex = (pitchHistoryWritePos - pitchWindowSamples + pitchHistoryCapacity) % pitchHistoryCapacity;

    double mean = 0.0;
    for (int i = 0; i < pitchWindowSamples; ++i)
    {
        const auto historyIndex = (firstIndex + i) % pitchHistoryCapacity;
        const auto sample = pitchHistory[historyIndex];
        pitchScratch[i] = sample;
        mean += sample;
    }

    mean /= static_cast<double> (pitchWindowSamples);

    double energy = 0.0;
    for (int i = 0; i < pitchWindowSamples; ++i)
    {
        const auto phase = static_cast<float> (juce::MathConstants<double>::twoPi * i
                                               / juce::jmax (1, pitchWindowSamples - 1));
        const auto window = 0.5f * (1.0f - std::cos (phase));
        const auto centred = static_cast<float> ((pitchScratch[i] - mean) * window);
        pitchScratch[i] = centred;
        energy += centred * centred;
    }

    if (energy <= 1.0e-8)
        return 0.0f;

    const auto minLag = juce::jlimit (1, pitchWindowSamples - 2,
                                      static_cast<int> (std::floor (currentSampleRate / maxPitchHz)));
    const auto maxLag = juce::jlimit (minLag + 1, pitchWindowSamples - 2,
                                      static_cast<int> (std::ceil (currentSampleRate / minPitchHz)));

    juce::HeapBlock<float> correlations;
    correlations.malloc (static_cast<size_t> (maxLag + 1));
    juce::FloatVectorOperations::clear (correlations.getData(), maxLag + 1);

    float bestCorrelation = 0.0f;
    int bestLag = 0;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double numerator = 0.0;
        double energyA = 0.0;
        double energyB = 0.0;
        const auto compareSamples = pitchWindowSamples - lag;

        for (int i = 0; i < compareSamples; ++i)
        {
            const auto a = pitchScratch[i];
            const auto b = pitchScratch[i + lag];
            numerator += static_cast<double> (a * b);
            energyA += static_cast<double> (a * a);
            energyB += static_cast<double> (b * b);
        }

        const auto correlation = safeNormalisedCorrelation (numerator, energyA, energyB);
        correlations[lag] = correlation;

        if (correlation > bestCorrelation)
        {
            bestCorrelation = correlation;
            bestLag = lag;
        }
    }


    if (bestLag <= 0 || bestCorrelation < minPitchCorrelation)
        return 0.0f;

    const auto nearPeakThreshold = juce::jmax (minPitchCorrelation,
                                               bestCorrelation * nearPeakCorrelationRatio);

    int selectedLag = bestLag;

    for (int lag = maxLag - 1; lag >= minLag + 1; --lag)
    {
        const auto correlation = correlations[lag];
        const auto isLocalPeak = correlation >= correlations[lag - 1]
                                 && correlation >= correlations[lag + 1];

        if (isLocalPeak && correlation >= nearPeakThreshold)
        {
            selectedLag = lag;
            break;
        }
    }

    const auto hz = static_cast<float> (currentSampleRate / static_cast<double> (selectedLag));
    return juce::jlimit (minPitchHz, maxPitchHz, hz);
}

float AudioAnalyzer::pitchHzToNormalised (float hz) noexcept
{
    const auto clampedHz = juce::jlimit (gamePitchMinHz, gamePitchMaxHz, hz);
    const auto minLog = std::log (gamePitchMinHz);
    const auto maxLog = std::log (gamePitchMaxHz);
    const auto hzLog = std::log (clampedHz);

    return juce::jlimit (0.0f, 1.0f,
                         static_cast<float> ((hzLog - minLog)
                                             / juce::jmax (1.0e-6f, maxLog - minLog)));
}

void AudioAnalyzer::recalculateThresholdsLocked() noexcept
{
    const auto baseNoiseFloor = noiseFloorDb.load();
    const auto offset = sensitivityOffsetDb.load();

    const auto open = juce::jlimit (-80.0f, -5.0f, baseNoiseFloor + openMarginDb + offset);
    const auto close = juce::jlimit (-83.0f, -8.0f, baseNoiseFloor + closeMarginDb + offset);

    openThresholdDb.store (open);
    closeThresholdDb.store (juce::jmin (close, open - 1.0f));
}

void AudioAnalyzer::finishCalibrationLocked() noexcept
{
    const auto averageDb = calibrationBlockCount > 0
                             ? static_cast<float> (calibrationDbSum / static_cast<double> (calibrationBlockCount))
                             : -55.0f;

    noiseFloorDb.store (juce::jlimit (-80.0f, -20.0f, averageDb + 1.5f));
    recalculateThresholdsLocked();

    calibrating.store (false);
    calibrationProgress.store (1.0f);
    intensityNormalised.store (0.0f);
    pitchHz.store (0.0f);
    pitchNormalised.store (0.0f);
    soundDetected.store (false);
    aboveThresholdSec = 0.0;
    belowThresholdSec = 0.0;

    resetPitchTrackingLocked();
}

void AudioAnalyzer::resetPitchTrackingLocked() noexcept
{
    pitchHz.store (0.0f);
    pitchNormalised.store (0.0f);
    recentPitchCandidates[0] = 0.0f;
    recentPitchCandidates[1] = 0.0f;
    recentPitchCandidates[2] = 0.0f;
    recentPitchCandidateCount = 0;
    recentPitchCandidateIndex = 0;
    lastRawCandidatePitchHz = 0.0f;
    consistentPitchBlocks = 0;
    invalidPitchBlocks = 0;
    pitchHistoryWritePos = 0;
    pitchHistoryCount = 0;

    if (pitchHistoryCapacity > 0)
        juce::FloatVectorOperations::clear (pitchHistory.getData(), pitchHistoryCapacity);
}
