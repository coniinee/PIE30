#include "TreeScene.h"
#include "SceneChrome.h"
#include <algorithm>

namespace
{
    juce::Colour bulbColourForIndex (int index)
    {
        static const juce::Colour colours[]
        {
            juce::Colour::fromRGB (255, 99, 132),
            juce::Colour::fromRGB (255, 206, 86),
            juce::Colour::fromRGB (75, 192, 192),
            juce::Colour::fromRGB (54, 162, 235),
            juce::Colour::fromRGB (153, 102, 255),
            juce::Colour::fromRGB (255, 159, 64)
        };

        return colours[index % static_cast<int> (std::size (colours))];
    }

    juce::Path makeStarPath (juce::Point<float> centre, float outerRadius, float innerRadius)
    {
        juce::Path star;

        for (int i = 0; i < 10; ++i)
        {
            const auto angle = juce::MathConstants<float>::halfPi + i * juce::MathConstants<float>::pi / 5.0f;
            const auto radius = (i % 2 == 0) ? outerRadius : innerRadius;
            const juce::Point<float> p { centre.x + std::cos (angle) * radius,
                                         centre.y - std::sin (angle) * radius };

            if (i == 0)
                star.startNewSubPath (p);
            else
                star.lineTo (p);
        }

        star.closeSubPath();
        return star;
    }
}

void TreeScene::setSceneData (GameSession::State newState,
                              float newProgress,
                              float newTargetProgress,
                              bool newSoundDetected,
                              float newCelebrateTime,
                              float newCalibrationProgress,
                              float newCountdownRemaining,
                              int newTotalStars,
                              int newLastStars,
                              const juce::String& newStageText,
                              const juce::String& newStatusText)
{
    state = newState;
    progress = juce::jlimit (0.0f, 1.0f, newProgress);
    targetProgress = juce::jlimit (0.05f, 1.0f, newTargetProgress);
    soundDetected = newSoundDetected;
    celebrateTime = newCelebrateTime;
    calibrationProgress = juce::jlimit (0.0f, 1.0f, newCalibrationProgress);
    countdownRemaining = juce::jmax (0.0f, newCountdownRemaining);
    totalStars = juce::jmax (0, newTotalStars);
    lastStars = juce::jmax (0, newLastStars);
    stageText = newStageText;
    statusText = newStatusText;
    repaint();
}

void TreeScene::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient sky (juce::Colour::fromRGB (9, 18, 43), bounds.getCentreX(), bounds.getBottom(),
                              juce::Colour::fromRGB (29, 89, 146), bounds.getCentreX(), bounds.getY(), false);
    sky.addColour (0.52, juce::Colour::fromRGB (14, 45, 83));
    g.setGradientFill (sky);
    g.fillAll();

    for (int i = 0; i < 18; ++i)
    {
        const float px = 70.0f + std::fmod (static_cast<float> (i) * 113.0f, juce::jmax (200.0f, bounds.getWidth() - 140.0f));
        const float py = 74.0f + std::fmod (static_cast<float> (i * 67), juce::jmax (220.0f, bounds.getHeight() - 220.0f)) * 0.62f;
        const float r = 1.8f + static_cast<float> (i % 3);
        g.setColour (juce::Colours::white.withAlpha (0.16f + 0.04f * static_cast<float> (i % 4)));
        g.fillEllipse ({ px, py, r, r });
    }

    auto ground = bounds.removeFromBottom (bounds.getHeight() * 0.145f);
    g.setColour (juce::Colour::fromRGBA (220, 232, 245, 236));
    g.fillRoundedRectangle (ground.translated (0.0f, 10.0f), 28.0f);

    SceneChrome::drawTopHud (g, getLocalBounds().toFloat(), "Guirlande du sapin", stageText, totalStars, lastStars);

    auto treeArea = getLocalBounds().toFloat().reduced (54.0f, 66.0f);
    treeArea.removeFromTop (70.0f);
    treeArea.removeFromBottom (74.0f);
    treeArea.removeFromLeft (28.0f);
    treeArea.removeFromRight (34.0f);

    const auto centreX = treeArea.getCentreX();
    const auto topY = treeArea.getY() + 12.0f;
    const auto trunkTopY = treeArea.getBottom() - 120.0f;
    const auto trunkWidth = 58.0f;
    const auto trunkHeight = 102.0f;

    juce::ColourGradient glow (juce::Colour::fromRGBA (75, 164, 141, 74), centreX, treeArea.getCentreY(),
                               juce::Colour::fromRGBA (75, 164, 141, 0), centreX, treeArea.getBottom(), true);
    g.setGradientFill (glow);
    g.fillEllipse ({ centreX - 208.0f, topY - 10.0f, 416.0f, trunkTopY - topY + 120.0f });

    juce::Path foliage;
    foliage.addTriangle ({ centreX, topY },
                         { treeArea.getX() + treeArea.getWidth() * 0.31f, topY + 112.0f },
                         { treeArea.getRight() - treeArea.getWidth() * 0.31f, topY + 112.0f });
    foliage.addTriangle ({ centreX, topY + 64.0f },
                         { treeArea.getX() + treeArea.getWidth() * 0.19f, topY + 202.0f },
                         { treeArea.getRight() - treeArea.getWidth() * 0.19f, topY + 202.0f });
    foliage.addTriangle ({ centreX, topY + 142.0f },
                         { treeArea.getX() + treeArea.getWidth() * 0.07f, trunkTopY + 8.0f },
                         { treeArea.getRight() - treeArea.getWidth() * 0.07f, trunkTopY + 8.0f });

    g.setColour (juce::Colour::fromRGB (36, 141, 89));
    g.fillPath (foliage);
    g.setColour (juce::Colour::fromRGB (15, 102, 61));
    g.strokePath (foliage, juce::PathStrokeType (3.0f));

    g.setColour (juce::Colour::fromRGB (124, 84, 46));
    g.fillRoundedRectangle ({ centreX - trunkWidth * 0.5f, trunkTopY, trunkWidth, trunkHeight }, 11.0f);

    const auto starPulse = 1.0f + 0.08f * std::sin (celebrateTime * 7.0f);
    auto star = makeStarPath ({ centreX, topY - 12.0f }, 24.0f * starPulse, 11.0f * starPulse);

    const bool successState = state == GameSession::State::roundSuccess || state == GameSession::State::programComplete;

    g.setColour (successState ? juce::Colour::fromRGBA (255, 244, 163, 84)
                              : juce::Colour::fromRGBA (255, 255, 255, 28));
    g.fillEllipse ({ centreX - 46.0f, topY - 56.0f, 92.0f, 92.0f });

    if (successState)
        g.setColour (juce::Colour::fromRGB (255, 215, 64));
    else
        g.setColour (juce::Colour::fromRGB (206, 208, 214));

    g.fillPath (star);

    const juce::Rectangle<float> garlandArea { treeArea.getX() + 36.0f,
                                               topY + 34.0f,
                                               treeArea.getWidth() - 72.0f,
                                               trunkTopY - topY - 6.0f };

    auto garland = makeGarlandPath (garlandArea);
    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.strokePath (garland, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    constexpr int bulbCount = 12;
    const auto bulbPositions = makeBulbPositions (garlandArea, bulbCount);
    const auto litCount = juce::jlimit (0, bulbCount,
                                        static_cast<int> (std::floor (progress * static_cast<float> (bulbCount) + 0.0001f)));

    for (int i = 0; i < bulbCount; ++i)
    {
        const auto pos = bulbPositions[static_cast<size_t> (i)];
        const auto bulbRadius = successState ? 10.8f : 9.2f;
        const auto bulbBounds = juce::Rectangle<float> (bulbRadius * 2.0f, bulbRadius * 2.0f).withCentre (pos);

        if (i < litCount)
        {
            const auto bulbColour = bulbColourForIndex (i);
            auto highlight = bulbBounds.withSizeKeepingCentre (bulbRadius * 0.92f, bulbRadius * 0.92f)
                                       .translated (-bulbRadius * 0.22f, -bulbRadius * 0.22f);

            g.setColour (bulbColour.withAlpha (soundDetected ? 0.44f : 0.22f));
            g.fillEllipse (bulbBounds.expanded (10.5f));
            g.setColour (bulbColour);
            g.fillEllipse (bulbBounds);
            g.setColour (juce::Colours::white.withAlpha (0.52f));
            g.fillEllipse (highlight);
        }
        else
        {
            g.setColour (juce::Colour::fromRGBA (86, 102, 126, 196));
            g.fillEllipse (bulbBounds);
            g.setColour (juce::Colour::fromRGBA (255, 255, 255, 18));
            g.drawEllipse (bulbBounds, 1.0f);
        }
    }

    const auto goalRatio = targetProgress > 0.0f ? juce::jlimit (0.0f, 1.0f, progress / targetProgress) : 0.0f;

    SceneChrome::StatusCardModel cardModel;
    switch (state)
    {
        case GameSession::State::idle:
            cardModel.badgeText = u8s("PRÊT");
            cardModel.badgeColour = juce::Colour::fromRGBA (120, 188, 255, 208);
            cardModel.headline = u8s("Appuyez sur Démarrer");
            break;
        case GameSession::State::calibrating:
            cardModel.badgeText = "CALIBRAGE";
            cardModel.badgeColour = juce::Colour::fromRGBA (120, 188, 255, 220);
            cardModel.headline = "Calibration du microphone";
            break;
        case GameSession::State::countdown:
            cardModel.badgeText = u8s("DÉCOMPTE");
            cardModel.badgeColour = juce::Colour::fromRGBA (146, 210, 255, 220);
            cardModel.headline = u8s("Préparez-vous");
            break;
        case GameSession::State::playing:
            cardModel.badgeText = soundDetected ? juce::String ("ACTIF") : u8s("ÉCOUTE");
            cardModel.badgeColour = soundDetected ? juce::Colour::fromRGBA (73, 214, 164, 220)
                                                  : juce::Colour::fromRGBA (255, 255, 255, 52);
            cardModel.headline = soundDetected ? "Gardez une voix stable"
                                               : "Faites un son stable";
            break;
        case GameSession::State::roundSuccess:
            cardModel.badgeText = u8s("ÉTAPE VALIDÉE");
            cardModel.badgeColour = juce::Colour::fromRGBA (255, 205, 72, 214);
            cardModel.headline = u8s("Étape validée");
            break;
        case GameSession::State::programComplete:
            cardModel.badgeText = u8s("TERMINÉ");
            cardModel.badgeColour = juce::Colour::fromRGBA (255, 205, 72, 214);
            cardModel.headline = u8s("Programme terminé");
            break;
    }

    cardModel.body = statusText;

    if (state == GameSession::State::calibrating)
    {
        cardModel.progressValue = calibrationProgress;
        cardModel.progressColour = juce::Colour::fromRGB (132, 205, 255);
        cardModel.footerLeft = u8s("Réglage micro");
        cardModel.footerCentre = juce::String (calibrationProgress * 100.0f, 0) + "%";
        cardModel.footerRight = "Silence";
    }
    else if (state == GameSession::State::countdown)
    {
        cardModel.progressValue = juce::jlimit (0.0f, 1.0f, 1.0f - countdownRemaining / 3.0f);
        cardModel.progressColour = juce::Colour::fromRGB (132, 205, 255);
        cardModel.footerLeft = u8s("Prêt");
        cardModel.footerCentre = juce::String (juce::jmax (1, static_cast<int> (std::ceil (countdownRemaining)))) + " s";
        cardModel.footerRight = u8s("Départ imminent");
    }
    else
    {
        cardModel.progressValue = goalRatio;
        cardModel.progressColour = successState ? juce::Colour::fromRGB (255, 205, 72)
                                                : juce::Colour::fromRGB (97, 218, 251);
        cardModel.footerLeft = "Objectif " + juce::String (goalRatio * 100.0f, 0) + "%";
        cardModel.footerCentre = u8s("Lumières ") + juce::String (progress * 100.0f, 0) + "%";
        cardModel.footerRight = successState ? juce::String ("Bravo")
                                             : (soundDetected ? juce::String ("Voix active")
                                                              : juce::String (u8s("Écoute")));
    }

    const auto cardWidth = juce::jlimit (430.0f, 560.0f, getWidth() * 0.50f);
    juce::Rectangle<float> card (centreX - cardWidth * 0.5f, getHeight() - 138.0f, cardWidth, 118.0f);
    SceneChrome::drawStatusCard (g, card, cardModel);

    if (successState)
    {
        const auto sparkleAlpha = 0.45f + 0.20f * std::sin (celebrateTime * 9.0f);
        g.setColour (juce::Colours::white.withAlpha (sparkleAlpha));

        for (int i = 0; i < 18; ++i)
        {
            const auto angle = static_cast<float> (i) * 0.35f + celebrateTime * 1.4f;
            const auto radius = 160.0f + 15.0f * std::sin (celebrateTime * 5.0f + static_cast<float> (i));
            juce::Point<float> p { centreX + std::cos (angle) * radius,
                                   topY + 118.0f + std::sin (angle) * radius * 0.55f };
            g.fillEllipse ({ p.x, p.y, 5.5f, 5.5f });
        }
    }

    if (state == GameSession::State::countdown)
        SceneChrome::drawCountdownOverlay (g, getLocalBounds().toFloat(), countdownRemaining,
                                           juce::Colour::fromRGBA (120, 188, 255, 210),
                                           u8s("Préparez votre voix"),
                                           "Commencez par un son stable");
}

std::vector<juce::Point<float>> TreeScene::makeBulbPositions (const juce::Rectangle<float>& treeArea, int bulbCount)
{
    std::vector<juce::Point<float>> anchors
    {
        { treeArea.getX() + treeArea.getWidth() * 0.34f, treeArea.getY() + treeArea.getHeight() * 0.16f },
        { treeArea.getX() + treeArea.getWidth() * 0.72f, treeArea.getY() + treeArea.getHeight() * 0.28f },
        { treeArea.getX() + treeArea.getWidth() * 0.25f, treeArea.getY() + treeArea.getHeight() * 0.46f },
        { treeArea.getX() + treeArea.getWidth() * 0.76f, treeArea.getY() + treeArea.getHeight() * 0.62f },
        { treeArea.getX() + treeArea.getWidth() * 0.22f, treeArea.getY() + treeArea.getHeight() * 0.80f }
    };

    std::vector<float> segmentLengths;
    segmentLengths.reserve (anchors.size() - 1);

    float totalLength = 0.0f;
    for (size_t i = 0; i + 1 < anchors.size(); ++i)
    {
        const auto length = anchors[i].getDistanceFrom (anchors[i + 1]);
        segmentLengths.push_back (length);
        totalLength += length;
    }

    std::vector<juce::Point<float>> result;
    result.reserve (static_cast<size_t> (bulbCount));

    for (int bulb = 0; bulb < bulbCount; ++bulb)
    {
        const auto target = totalLength * static_cast<float> (bulb + 0.5f) / static_cast<float> (bulbCount);
        float travelled = 0.0f;

        for (size_t seg = 0; seg + 1 < anchors.size(); ++seg)
        {
            const auto segLength = segmentLengths[seg];
            if (target <= travelled + segLength || seg == anchors.size() - 2)
            {
                const auto localT = juce::jlimit (0.0f, 1.0f, (target - travelled) / juce::jmax (1.0f, segLength));
                result.push_back (anchors[seg] + (anchors[seg + 1] - anchors[seg]) * localT);
                break;
            }

            travelled += segLength;
        }
    }

    std::reverse (result.begin(), result.end());
    return result;
}

juce::Path TreeScene::makeGarlandPath (const juce::Rectangle<float>& treeArea)
{
    juce::Path path;
    const std::vector<juce::Point<float>> anchors
    {
        { treeArea.getX() + treeArea.getWidth() * 0.34f, treeArea.getY() + treeArea.getHeight() * 0.16f },
        { treeArea.getX() + treeArea.getWidth() * 0.72f, treeArea.getY() + treeArea.getHeight() * 0.28f },
        { treeArea.getX() + treeArea.getWidth() * 0.25f, treeArea.getY() + treeArea.getHeight() * 0.46f },
        { treeArea.getX() + treeArea.getWidth() * 0.76f, treeArea.getY() + treeArea.getHeight() * 0.62f },
        { treeArea.getX() + treeArea.getWidth() * 0.22f, treeArea.getY() + treeArea.getHeight() * 0.80f }
    };

    path.startNewSubPath (anchors.front());
    for (size_t i = 1; i < anchors.size(); ++i)
        path.lineTo (anchors[i]);

    return path;
}
