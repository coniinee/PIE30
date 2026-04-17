#pragma once

#include <JuceHeader.h>
#include "Utf8Text.h"
#include <algorithm>
#include <cmath>

class GameSession
{
public:
    enum class State
    {
        idle,
        calibrating,
        countdown,
        playing,
        roundSuccess,
        programComplete
    };

    enum class Mode
    {
        practice,
        guidedProgram
    };

    enum class Difficulty
    {
        easy,
        standard,
        advanced
    };

    enum class GameType
    {
        treeLights,
        windmill,
        butterflyPitch
    };

    struct StageGoal
    {
        juce::String title;
        juce::String instruction;
        float targetProgress = 1.0f;
        float riseSpeed = 0.22f;
        float fallSpeed = 0.08f;
        float successHoldRequired = 0.5f;
        float targetWindow = 0.0f;
    };

    void reset()
    {
        state = State::idle;
        currentStageIndex = 0;
        progress = 0.0f;
        successHoldTimer = 0.0f;
        celebrateTimer = 0.0f;
        countdownRemaining = defaultCountdownSeconds;
        currentStreak = 0.0f;
        roundLongestStreak = 0.0f;
        longestStreakOverall = 0.0f;
        totalVoicedTime = 0.0f;
        roundVoicedTime = 0.0f;
        roundElapsedTime = 0.0f;
        roundVoiceSegments = 0;
        totalStars = 0;
        lastAwardedStars = 0;
        previousSoundDetected = false;
    }

    void setGameType (GameType newGameType)
    {
        gameType = newGameType;
        reset();
    }

    void setMode (Mode newMode)
    {
        mode = newMode;
        reset();
    }

    void setDifficulty (Difficulty newDifficulty)
    {
        difficulty = newDifficulty;
        reset();
    }

    void setCountdownEnabled (bool shouldEnable) noexcept
    {
        countdownEnabled = shouldEnable;
    }

    void startCalibration()
    {
        if (state == State::roundSuccess && mode == Mode::guidedProgram && currentStageIndex + 1 < getStageCount())
            ++currentStageIndex;
        else if (state == State::programComplete)
            currentStageIndex = 0;
        else if (state == State::idle)
            currentStageIndex = 0;

        if (state == State::idle || state == State::programComplete)
        {
            totalVoicedTime = 0.0f;
            longestStreakOverall = 0.0f;
            totalStars = 0;
        }

        resetRoundOnly();
        state = State::calibrating;
    }

    void update (float dtSeconds,
                 bool soundDetected,
                 bool analyzerIsCalibrating,
                 float intensityNormalised,
                 float pitchNormalised)
    {
        dtSeconds = juce::jlimit (0.0f, 0.25f, dtSeconds);
        const auto goal = getCurrentGoal();
        const auto clampedIntensity = juce::jlimit (0.0f, 1.0f, intensityNormalised);
        const auto clampedPitch = juce::jlimit (0.0f, 1.0f, pitchNormalised);
        const auto validPitchControl = soundDetected && clampedPitch > 0.005f;

        switch (state)
        {
            case State::idle:
                break;

            case State::calibrating:
                if (! analyzerIsCalibrating)
                {
                    if (countdownEnabled)
                    {
                        state = State::countdown;
                        countdownRemaining = defaultCountdownSeconds;
                    }
                    else
                    {
                        state = State::playing;
                    }
                }
                break;

            case State::countdown:
                countdownRemaining = juce::jmax (0.0f, countdownRemaining - dtSeconds);
                if (countdownRemaining <= 0.0f)
                    state = State::playing;
                break;

            case State::playing:
            {
                roundElapsedTime += dtSeconds;

                if (soundDetected)
                {
                    currentStreak += dtSeconds;
                    roundLongestStreak = juce::jmax (roundLongestStreak, currentStreak);
                    longestStreakOverall = juce::jmax (longestStreakOverall, currentStreak);
                    roundVoicedTime += dtSeconds;
                    totalVoicedTime += dtSeconds;

                    if (! previousSoundDetected)
                        ++roundVoiceSegments;
                }
                else
                {
                    currentStreak = 0.0f;
                }

                previousSoundDetected = soundDetected;

                if (gameType == GameType::treeLights)
                {
                    const auto delta = soundDetected ? goal.riseSpeed * dtSeconds
                                                     : -goal.fallSpeed * dtSeconds;
                    progress = juce::jlimit (0.0f, goal.targetProgress, progress + delta);
                }
                else if (gameType == GameType::windmill)
                {
                    float delta = 0.0f;

                    if (soundDetected)
                    {
                        const auto weightedIntensity = 0.30f + 0.95f * clampedIntensity;
                        delta = goal.riseSpeed * weightedIntensity * dtSeconds;
                    }
                    else
                    {
                        delta = -goal.fallSpeed * dtSeconds;
                    }

                    progress = juce::jlimit (0.0f, goal.targetProgress, progress + delta);
                }
                else
                {
                    if (validPitchControl)
                    {
                        const auto desired = clampedPitch;
                        const auto moveRate = goal.riseSpeed * dtSeconds;
                        const auto downwardMoveRate = (goal.riseSpeed + 0.18f) * dtSeconds;
                        const auto delta = desired - progress;

                        if (std::abs (delta) <= moveRate)
                        {
                            progress = desired;
                        }
                        else if (delta > 0.0f)
                        {
                            progress = juce::jmin (1.0f, progress + moveRate);
                        }
                        else
                        {
                            progress = juce::jmax (0.0f, progress - juce::jmin (downwardMoveRate, std::abs (delta)));
                        }
                    }
                    else
                    {
                        progress = juce::jmax (0.0f, progress - goal.fallSpeed * dtSeconds);
                    }
                }

                const auto isPitchBandGoal = goal.targetWindow > 0.0f;
                const auto withinGoal = isPitchBandGoal
                                          ? validPitchControl && std::abs (progress - goal.targetProgress) <= goal.targetWindow
                                          : progress >= goal.targetProgress - 0.001f;

                if (withinGoal)
                    successHoldTimer += dtSeconds;
                else
                    successHoldTimer = 0.0f;

                if (successHoldTimer >= goal.successHoldRequired)
                {
                    lastAwardedStars = computeRoundStars (goal);
                    totalStars += lastAwardedStars;
                    celebrateTimer = 0.0f;
                    currentStreak = 0.0f;

                    if (mode == Mode::guidedProgram && currentStageIndex + 1 >= getStageCount())
                        state = State::programComplete;
                    else
                        state = State::roundSuccess;
                }

                break;
            }

            case State::roundSuccess:
            case State::programComplete:
                celebrateTimer += dtSeconds;
                break;
        }
    }

    [[nodiscard]] State getState() const noexcept                 { return state; }
    [[nodiscard]] Mode getMode() const noexcept                   { return mode; }
    [[nodiscard]] Difficulty getDifficulty() const noexcept       { return difficulty; }
    [[nodiscard]] GameType getGameType() const noexcept           { return gameType; }
    [[nodiscard]] bool isCountdownEnabled() const noexcept        { return countdownEnabled; }

    [[nodiscard]] float getProgress() const noexcept              { return progress; }
    [[nodiscard]] float getTargetProgress() const noexcept        { return getCurrentGoal().targetProgress; }
    [[nodiscard]] float getCelebrateTime() const noexcept         { return celebrateTimer; }
    [[nodiscard]] float getCountdownRemaining() const noexcept    { return countdownRemaining; }
    [[nodiscard]] float getCurrentStreak() const noexcept         { return currentStreak; }
    [[nodiscard]] float getLongestStreak() const noexcept         { return longestStreakOverall; }
    [[nodiscard]] float getRoundVoicedTime() const noexcept       { return roundVoicedTime; }
    [[nodiscard]] float getTotalVoicedTime() const noexcept       { return totalVoicedTime; }
    [[nodiscard]] float getRoundElapsedTime() const noexcept      { return roundElapsedTime; }
    [[nodiscard]] float getRoundStability() const noexcept        { return roundElapsedTime > 0.0f ? roundVoicedTime / roundElapsedTime : 0.0f; }
    [[nodiscard]] int getCurrentStageIndex() const noexcept       { return currentStageIndex; }
    [[nodiscard]] int getStageCount() const noexcept              { return mode == Mode::guidedProgram ? 3 : 1; }
    [[nodiscard]] int getTotalStars() const noexcept              { return totalStars; }
    [[nodiscard]] int getLastAwardedStars() const noexcept        { return lastAwardedStars; }
    [[nodiscard]] int getRoundVoiceSegments() const noexcept      { return roundVoiceSegments; }

    [[nodiscard]] StageGoal getCurrentGoal() const
    {
        const auto baseRise = difficulty == Difficulty::easy ? 0.26f
                              : difficulty == Difficulty::standard ? 0.22f
                                                                   : 0.19f;

        const auto baseFall = difficulty == Difficulty::easy ? 0.05f
                              : difficulty == Difficulty::standard ? 0.08f
                                                                   : 0.11f;

        const auto baseHold = difficulty == Difficulty::easy ? 0.35f
                              : difficulty == Difficulty::standard ? 0.50f
                                                                   : 0.70f;

        if (gameType == GameType::treeLights)
        {
            if (mode == Mode::practice)
            {
                return { u8s("Mode entraînement"),
                         "Allumez toute la guirlande et gardez une voix stable.",
                         1.0f,
                         baseRise,
                         baseFall,
                         baseHold,
                         0.0f };
            }

            switch (currentStageIndex)
            {
                case 0:
                    return { u8s("Étape 1 - Échauffement"),
                             u8s("Atteignez les lumières du bas avec une voix stable et brève."),
                             0.45f,
                             baseRise + 0.03f,
                             juce::jmax (0.03f, baseFall - 0.02f),
                             juce::jmax (0.20f, baseHold - 0.15f),
                             0.0f };

                case 1:
                    return { u8s("Étape 2 - Garder la stabilité"),
                             "Montez plus haut et maintenez le son plus longtemps.",
                             0.75f,
                             baseRise,
                             baseFall,
                             juce::jmax (0.30f, baseHold - 0.08f),
                             0.0f };

                default:
                    return { u8s("Étape 3 - Défi du sapin complet"),
                             u8s("Allumez toute la guirlande et gardez les lumières du haut allumées."),
                             1.0f,
                             juce::jmax (0.14f, baseRise - 0.02f),
                             baseFall + 0.01f,
                             baseHold + 0.10f,
                             0.0f };
            }
        }

        if (gameType == GameType::windmill)
        {
            if (mode == Mode::practice)
            {
                return { u8s("Mode entraînement"),
                         "Plus la voix est forte, plus les ailes tournent vite. Gardez le moulin en mouvement pour remplir la jauge.",
                         1.0f,
                         baseRise + 0.04f,
                         juce::jmax (0.03f, baseFall - 0.01f),
                         juce::jmax (0.25f, baseHold - 0.05f),
                         0.0f };
            }

            switch (currentStageIndex)
            {
                case 0:
                    return { u8s("Étape 1 - Brise légère"),
                             "Lancez les ailes avec une voix douce mais stable.",
                             0.42f,
                             baseRise + 0.02f,
                             juce::jmax (0.03f, baseFall - 0.03f),
                             juce::jmax (0.18f, baseHold - 0.18f),
                             0.0f };

                case 1:
                    return { u8s("Étape 2 - Vent régulier"),
                             u8s("Gardez les ailes en rotation avec une voix moyenne et régulière."),
                             0.74f,
                             baseRise + 0.05f,
                             baseFall,
                             juce::jmax (0.28f, baseHold - 0.10f),
                             0.0f };

                default:
                    return { u8s("Étape 3 - Défi du vent fort"),
                             "Utilisez une voix plus forte pour tourner vite et garder la jauge pleine.",
                             1.0f,
                             baseRise + 0.08f,
                             baseFall + 0.01f,
                             baseHold,
                             0.0f };
            }
        }

        const auto baseWindow = difficulty == Difficulty::easy ? 0.20f
                                : difficulty == Difficulty::standard ? 0.16f
                                                                     : 0.12f;
        const auto baseTrackRate = difficulty == Difficulty::easy ? 1.14f
                                   : difficulty == Difficulty::standard ? 1.00f
                                                                        : 0.88f;
        const auto baseReturnRate = difficulty == Difficulty::easy ? 0.20f
                                    : difficulty == Difficulty::standard ? 0.25f
                                                                         : 0.31f;
        const auto basePitchHold = difficulty == Difficulty::easy ? 0.24f
                                   : difficulty == Difficulty::standard ? 0.34f
                                                                        : 0.48f;

        if (mode == Mode::practice)
        {
            return { u8s("Mode entraînement"),
                     u8s("Faites monter ou descendre le papillon avec la hauteur de votre voix et maintenez-le dans l’anneau arc-en-ciel."),
                     0.48f,
                     baseTrackRate,
                     baseReturnRate,
                     basePitchHold,
                     baseWindow + 0.04f };
        }

        switch (currentStageIndex)
        {
            case 0:
                return { u8s("Étape 1 - Fleur basse"),
                         u8s("Utilisez une hauteur plus grave et confortable pour garder le papillon près de la première fleur."),
                         0.18f,
                         baseTrackRate + 0.08f,
                         juce::jmax (0.16f, baseReturnRate - 0.04f),
                         juce::jmax (0.18f, basePitchHold - 0.06f),
                         baseWindow + 0.04f };

            case 1:
                return { u8s("Étape 2 - Fleur centrale"),
                         u8s("Montez légèrement la hauteur et gardez le papillon près de la fleur centrale."),
                         0.42f,
                         baseTrackRate,
                         baseReturnRate,
                         basePitchHold,
                         baseWindow + 0.01f };

            default:
                return { u8s("Étape 3 - Fleur haute"),
                         u8s("Utilisez une hauteur nettement plus aiguë et gardez le papillon près de la fleur du haut."),
                         0.70f,
                         juce::jmax (0.76f, baseTrackRate - 0.04f),
                         baseReturnRate + 0.02f,
                         basePitchHold + 0.06f,
                         juce::jmax (0.09f, baseWindow - 0.01f) };
        }
    }

    [[nodiscard]] juce::String getGameTypeName() const
    {
        switch (gameType)
        {
            case GameType::treeLights:    return "Guirlande du sapin";
            case GameType::windmill:      return u8s("Moulin à vent");
            case GameType::butterflyPitch:return "Vol du papillon";
        }

        return "Guirlande du sapin";
    }

    [[nodiscard]] juce::String getModeName() const
    {
        return mode == Mode::practice ? u8s("Entraînement") : u8s("Programme guidé");
    }

    [[nodiscard]] juce::String getDifficultyName() const
    {
        switch (difficulty)
        {
            case Difficulty::easy:     return "Facile";
            case Difficulty::standard: return "Standard";
            case Difficulty::advanced: return u8s("Avancé");
        }

        return "Standard";
    }

    [[nodiscard]] juce::String getPrimaryActionText() const
    {
        switch (state)
        {
            case State::idle:
                return mode == Mode::practice ? u8s("Démarrer l’entraînement") : u8s("Démarrer le programme");

            case State::calibrating:
            case State::countdown:
            case State::playing:
                return "Recalibrer";

            case State::roundSuccess:
                return mode == Mode::guidedProgram ? u8s("Étape suivante") : juce::String ("Rejouer");

            case State::programComplete:
                return u8s("Redémarrer le programme");
        }

        return u8s("Démarrer");
    }

private:
    void resetRoundOnly() noexcept
    {
        state = State::idle;
        progress = 0.0f;
        successHoldTimer = 0.0f;
        celebrateTimer = 0.0f;
        countdownRemaining = defaultCountdownSeconds;
        currentStreak = 0.0f;
        roundLongestStreak = 0.0f;
        roundVoicedTime = 0.0f;
        roundElapsedTime = 0.0f;
        roundVoiceSegments = 0;
        lastAwardedStars = 0;
        previousSoundDetected = false;
    }

    int computeRoundStars (const StageGoal& goal) const noexcept
    {
        auto estimatedContinuousVoice = goal.targetProgress / juce::jmax (0.05f, goal.riseSpeed)
                                      + goal.successHoldRequired;

        if (gameType == GameType::butterflyPitch)
            estimatedContinuousVoice = 1.0f + goal.targetProgress * 0.7f + goal.successHoldRequired;

        int stars = 1;

        if (roundLongestStreak >= estimatedContinuousVoice * 0.70f)
            ++stars;

        if (roundVoiceSegments <= (mode == Mode::guidedProgram ? 2 : 3))
            ++stars;

        return juce::jlimit (1, 3, stars);
    }

    GameType gameType = GameType::treeLights;
    Mode mode = Mode::guidedProgram;
    Difficulty difficulty = Difficulty::standard;
    bool countdownEnabled = true;

    State state = State::idle;
    int currentStageIndex = 0;

    float progress = 0.0f;
    float successHoldTimer = 0.0f;
    float celebrateTimer = 0.0f;
    float countdownRemaining = defaultCountdownSeconds;

    float currentStreak = 0.0f;
    float roundLongestStreak = 0.0f;
    float longestStreakOverall = 0.0f;
    float totalVoicedTime = 0.0f;
    float roundVoicedTime = 0.0f;
    float roundElapsedTime = 0.0f;

    int roundVoiceSegments = 0;
    int totalStars = 0;
    int lastAwardedStars = 0;

    bool previousSoundDetected = false;

    static constexpr float defaultCountdownSeconds = 3.0f;
};
