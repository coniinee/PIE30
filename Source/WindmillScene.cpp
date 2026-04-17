#include "WindmillScene.h"
#include "SceneChrome.h"

namespace
{
    juce::Path makeBladePath (float length, float width)
    {
        juce::Path p;
        p.startNewSubPath (0.0f, -width * 0.20f);
        p.quadraticTo (length * 0.56f, -width, length, -width * 0.18f);
        p.lineTo (length * 0.92f, width * 0.18f);
        p.quadraticTo (length * 0.52f, width * 0.78f, 0.0f, width * 0.16f);
        p.closeSubPath();
        return p;
    }

    juce::Path makeArcPath (juce::Point<float> centre, float radius, float startAngle, float endAngle)
    {
        juce::Path p;
        p.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, startAngle, endAngle, true);
        return p;
    }

    void drawRewardFlower (juce::Graphics& g,
                           juce::Point<float> centre,
                           float size,
                           float bloom,
                           float time,
                           juce::Colour petalColour,
                           juce::Colour centreColour)
    {
        bloom = juce::jlimit (0.0f, 1.0f, bloom);
        const float pulse = 1.0f + 0.05f * std::sin (time * 5.0f + centre.x * 0.01f);
        const float alpha = 0.22f + 0.78f * bloom;
        const float petalDistance = size * juce::jmap (bloom, 0.0f, 1.0f, 0.16f, 0.52f) * pulse;
        const float petalW = size * juce::jmap (bloom, 0.0f, 1.0f, 0.40f, 0.68f) * pulse;
        const float petalH = size * juce::jmap (bloom, 0.0f, 1.0f, 0.56f, 0.90f) * pulse;

        g.setColour (juce::Colour::fromRGB (83, 146, 86).withAlpha (0.72f));
        g.drawLine (centre.x, centre.y + size * 0.34f,
                    centre.x, centre.y + size * 1.20f,
                    2.6f);
        g.setColour (juce::Colour::fromRGB (110, 177, 100).withAlpha (0.40f));
        g.fillEllipse (centre.x - size * 0.42f, centre.y + size * 0.62f, size * 0.34f, size * 0.20f);
        g.fillEllipse (centre.x + size * 0.06f, centre.y + size * 0.72f, size * 0.34f, size * 0.20f);

        g.setColour (petalColour.brighter (0.4f).withAlpha (0.08f + 0.12f * bloom));
        g.fillEllipse (centre.x - size * 0.90f, centre.y - size * 0.78f, size * 1.80f, size * 1.55f);

        for (int i = 0; i < 6; ++i)
        {
            const float a = juce::MathConstants<float>::twoPi * static_cast<float> (i) / 6.0f;
            const juce::Point<float> p (centre.x + std::cos (a) * petalDistance,
                                        centre.y + std::sin (a) * petalDistance);
            g.setColour (petalColour.withAlpha (alpha));
            g.fillEllipse (p.x - petalW * 0.5f, p.y - petalH * 0.5f, petalW, petalH);
        }

        g.setColour (centreColour.withAlpha (0.40f + 0.60f * bloom));
        g.fillEllipse (centre.x - size * 0.24f, centre.y - size * 0.24f, size * 0.48f, size * 0.48f);
    }

    void drawCelebrationSparkles (juce::Graphics& g,
                                  juce::Rectangle<float> area,
                                  float time)
    {
        for (int i = 0; i < 10; ++i)
        {
            const float t = time * 0.9f + static_cast<float> (i) * 0.61f;
            const float x = area.getX() + area.getWidth() * (0.08f + 0.84f * std::fmod (0.37f * static_cast<float> (i) + 0.12f, 1.0f));
            const float y = area.getY() + area.getHeight() * (0.20f + 0.55f * (0.5f + 0.5f * std::sin (t * 1.4f + static_cast<float> (i))));
            const float r = 2.0f + 1.5f * (0.5f + 0.5f * std::sin (t * 3.0f));
            g.setColour (juce::Colour::fromRGB (255, 235, 160).withAlpha (0.28f));
            g.fillEllipse (x - r * 2.5f, y - r * 2.5f, r * 5.0f, r * 5.0f);
            g.setColour (juce::Colour::fromRGB (255, 252, 224).withAlpha (0.82f));
            g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
        }
    }

    void drawWindProgressRing (juce::Graphics& g,
                               juce::Point<float> centre,
                               float progress01,
                               bool successState,
                               bool soundDetected,
                               float intensityNormalised,
                               float time)
    {
        constexpr int segmentCount = 5;
        constexpr float totalSpan = juce::MathConstants<float>::twoPi * 0.82f;
        constexpr float startAngle = -juce::MathConstants<float>::pi * 0.91f;
        constexpr float segmentGap = 0.11f;
        const float radius = 182.0f;
        const float strokeThickness = 13.0f;
        const float progressSegments = juce::jlimit (0.0f, 1.0f, progress01) * static_cast<float> (segmentCount);
        const float pulse = soundDetected ? (0.5f + 0.5f * std::sin (time * 5.5f)) : 0.0f;

        for (int i = 0; i < segmentCount; ++i)
        {
            const float segStart = startAngle + totalSpan * static_cast<float> (i) / static_cast<float> (segmentCount) + segmentGap * 0.5f;
            const float segEnd = startAngle + totalSpan * static_cast<float> (i + 1) / static_cast<float> (segmentCount) - segmentGap * 0.5f;
            auto baseArc = makeArcPath (centre, radius, segStart, segEnd);

            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.strokePath (baseArc, juce::PathStrokeType (strokeThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            const float fill = juce::jlimit (0.0f, 1.0f, progressSegments - static_cast<float> (i));
            if (fill <= 0.0f)
                continue;

            const float fillEnd = juce::jmap (fill, segStart, segEnd);
            auto fillArc = makeArcPath (centre, radius, segStart, fillEnd);

            juce::Colour fillColour;
            if (successState)
                fillColour = juce::Colour::fromRGB (255, 214, 88);
            else
                fillColour = juce::Colour::fromHSV (0.10f + 0.03f * static_cast<float> (i), 0.55f, 1.0f, 1.0f);

            const float glowAlpha = successState ? 0.30f : (0.12f + 0.16f * fill + 0.10f * pulse * intensityNormalised);
            g.setColour (fillColour.withAlpha (glowAlpha));
            g.strokePath (fillArc, juce::PathStrokeType (strokeThickness + 12.0f,
                                                         juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));

            g.setColour (fillColour.withAlpha (successState ? 0.98f : (0.68f + 0.24f * fill)));
            g.strokePath (fillArc, juce::PathStrokeType (strokeThickness,
                                                         juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));
        }

        if (soundDetected && ! successState)
        {
            const float swirlAlpha = 0.10f + 0.10f * intensityNormalised;
            for (int i = 0; i < 3; ++i)
            {
                const float r = radius - 24.0f - static_cast<float> (i) * 18.0f;
                const float a0 = -0.48f + time * (1.2f + 0.15f * static_cast<float> (i));
                const float a1 = a0 + 0.80f;
                auto swirl = makeArcPath (centre, r, a0, a1);
                g.setColour (juce::Colours::white.withAlpha (swirlAlpha * (1.0f - 0.18f * static_cast<float> (i))));
                g.strokePath (swirl, juce::PathStrokeType (3.0f,
                                                           juce::PathStrokeType::curved,
                                                           juce::PathStrokeType::rounded));
            }
        }
    }
}

void WindmillScene::setSceneData (GameSession::State newState,
                                  float newProgress,
                                  float newTargetProgress,
                                  bool newSoundDetected,
                                  float newIntensityNormalised,
                                  float newCelebrateTime,
                                  float newCalibrationProgress,
                                  float newCountdownRemaining,
                                  int newTotalStars,
                                  int newLastStars,
                                  const juce::String& newStageText,
                                  const juce::String& newStatusText)
{
    const auto nowMs = juce::Time::getMillisecondCounterHiRes();
    const auto dt = lastFrameMs > 0.0 ? static_cast<float> ((nowMs - lastFrameMs) / 1000.0) : 1.0f / 30.0f;
    lastFrameMs = nowMs;

    state = newState;
    progress = juce::jlimit (0.0f, 1.0f, newProgress);
    targetProgress = juce::jlimit (0.05f, 1.0f, newTargetProgress);
    soundDetected = newSoundDetected;
    intensityNormalised = juce::jlimit (0.0f, 1.0f, newIntensityNormalised);
    celebrateTime = newCelebrateTime;
    calibrationProgress = juce::jlimit (0.0f, 1.0f, newCalibrationProgress);
    countdownRemaining = juce::jmax (0.0f, newCountdownRemaining);
    totalStars = juce::jmax (0, newTotalStars);
    lastStars = juce::jmax (0, newLastStars);
    stageText = newStageText;
    statusText = newStatusText;

    const auto successState = state == GameSession::State::roundSuccess || state == GameSession::State::programComplete;
    float targetSpin = 0.0f;

    if (successState)
        targetSpin = 1.9f;
    else if (state == GameSession::State::countdown)
        targetSpin = 0.22f;
    else if (state == GameSession::State::playing && soundDetected)
        targetSpin = juce::jmap (intensityNormalised, 0.0f, 1.0f, 0.25f, 1.65f);

    displayedSpin += (targetSpin - displayedSpin) * juce::jlimit (0.0f, 1.0f, dt * 4.4f);
    bladeAngle = std::fmod (bladeAngle + displayedSpin * juce::MathConstants<float>::twoPi * dt,
                            juce::MathConstants<float>::twoPi);
    repaint();
}

void WindmillScene::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float visualTime = static_cast<float> (juce::Time::getMillisecondCounterHiRes() * 0.001);

    juce::ColourGradient sky (juce::Colour::fromRGB (27, 86, 150), bounds.getCentreX(), bounds.getBottom(),
                              juce::Colour::fromRGB (137, 214, 255), bounds.getCentreX(), bounds.getY(), false);
    sky.addColour (0.58, juce::Colour::fromRGB (76, 160, 223));
    g.setGradientFill (sky);
    g.fillAll();

    for (int i = 0; i < 4; ++i)
    {
        const float cloudX = 76.0f + static_cast<float> (i) * 212.0f + 22.0f * std::sin (visualTime * 0.35f + static_cast<float> (i) * 0.8f);
        const float cloudY = 80.0f + static_cast<float> (i % 2) * 46.0f;
        g.setColour (juce::Colours::white.withAlpha (0.22f));
        g.fillEllipse (cloudX, cloudY, 116.0f, 42.0f);
        g.fillEllipse (cloudX + 28.0f, cloudY - 12.0f, 64.0f, 52.0f);
        g.fillEllipse (cloudX + 58.0f, cloudY + 3.0f, 88.0f, 38.0f);
    }

    auto groundBand = bounds.removeFromBottom (bounds.getHeight() * 0.23f);
    g.setColour (juce::Colour::fromRGB (66, 153, 84));
    g.fillEllipse ({ -26.0f, groundBand.getY() - 12.0f, bounds.getWidth() * 0.63f, groundBand.getHeight() * 1.68f });
    g.fillEllipse ({ bounds.getWidth() * 0.25f, groundBand.getY() - 26.0f, bounds.getWidth() * 0.86f, groundBand.getHeight() * 1.78f });

    SceneChrome::drawTopHud (g, getLocalBounds().toFloat(), u8s("Moulin à vent"), stageText, totalStars, lastStars);

    const float cardWidth = juce::jlimit (430.0f, 560.0f, getWidth() * 0.50f);
    juce::Rectangle<float> infoCard (getWidth() * 0.5f - cardWidth * 0.5f,
                                     getHeight() - 138.0f,
                                     cardWidth,
                                     118.0f);

    auto sceneArea = getLocalBounds().toFloat().reduced (40.0f, 56.0f);
    sceneArea.removeFromTop (82.0f);
    sceneArea.removeFromBottom (76.0f);

    const auto centre = juce::Point<float> (sceneArea.getCentreX() + 8.0f, sceneArea.getCentreY() - 42.0f);
    const float towerHeight = 320.0f;
    const float towerWidthTop = 44.0f;
    const float towerWidthBottom = 106.0f;
    const float hubRadius = 22.0f;
    const auto successState = state == GameSession::State::roundSuccess || state == GameSession::State::programComplete;
    const float goalRatio = targetProgress > 0.0f ? juce::jlimit (0.0f, 1.0f, progress / targetProgress) : progress;

    juce::ColourGradient windGlow (juce::Colour::fromRGBA (255, 255, 255, 42), centre.x, centre.y,
                                   juce::Colour::fromRGBA (255, 255, 255, 0), centre.x, centre.y + 250.0f, true);
    g.setGradientFill (windGlow);
    g.fillEllipse ({ centre.x - 238.0f, centre.y - 126.0f, 476.0f, 476.0f });

    drawWindProgressRing (g, centre, successState ? 1.0f : goalRatio, successState, soundDetected, intensityNormalised, visualTime);

    juce::Path tower;
    tower.startNewSubPath (centre.x - towerWidthTop * 0.5f, centre.y + 18.0f);
    tower.lineTo (centre.x + towerWidthTop * 0.5f, centre.y + 18.0f);
    tower.lineTo (centre.x + towerWidthBottom * 0.5f, centre.y + towerHeight);
    tower.lineTo (centre.x - towerWidthBottom * 0.5f, centre.y + towerHeight);
    tower.closeSubPath();

    g.setColour (juce::Colour::fromRGB (228, 214, 185));
    g.fillPath (tower);
    g.setColour (juce::Colour::fromRGB (145, 114, 78));
    g.strokePath (tower, juce::PathStrokeType (2.5f));

    for (int i = 0; i < 6; ++i)
    {
        const float y = centre.y + 46.0f + static_cast<float> (i) * 36.0f;
        g.drawLine (centre.x - 28.0f, y, centre.x + 28.0f, y, 2.0f);
    }

    g.drawLine (centre.x - 20.0f, centre.y + 40.0f, centre.x + 30.0f, centre.y + towerHeight - 10.0f, 2.0f);
    g.drawLine (centre.x + 20.0f, centre.y + 40.0f, centre.x - 30.0f, centre.y + towerHeight - 10.0f, 2.0f);

    auto capRect = juce::Rectangle<float> (104.0f, 46.0f).withCentre ({ centre.x, centre.y + 5.0f });
    g.setColour (juce::Colour::fromRGB (184, 80, 58));
    g.fillRoundedRectangle (capRect, 12.0f);

    const auto bladePath = makeBladePath (158.0f, 31.0f);
    for (int i = 0; i < 4; ++i)
    {
        const auto angle = bladeAngle + static_cast<float> (i) * juce::MathConstants<float>::halfPi;
        juce::AffineTransform t = juce::AffineTransform::rotation (angle).translated (centre.x, centre.y);
        auto blade = bladePath;
        blade.applyTransform (t);

        g.setColour (successState ? juce::Colour::fromRGB (255, 238, 192)
                                  : juce::Colour::fromRGB (250, 247, 237));
        g.fillPath (blade);
        g.setColour (juce::Colour::fromRGB (164, 144, 110));
        g.strokePath (blade, juce::PathStrokeType (1.8f));
    }

    g.setColour (juce::Colour::fromRGB (106, 74, 50));
    g.fillEllipse ({ centre.x - hubRadius, centre.y - hubRadius, hubRadius * 2.0f, hubRadius * 2.0f });
    g.setColour (juce::Colours::white.withAlpha (0.26f));
    g.fillEllipse ({ centre.x - 8.0f, centre.y - 8.0f, 16.0f, 16.0f });

    const float towerBaseY = centre.y + towerHeight;
    g.setColour (juce::Colours::black.withAlpha (0.12f));
    g.fillEllipse ({ centre.x - 52.0f, towerBaseY - 10.0f, 104.0f, 18.0f });
    g.setColour (juce::Colours::black.withAlpha (0.05f));
    g.fillEllipse ({ centre.x - 76.0f, towerBaseY - 6.0f, 152.0f, 10.0f });

    if (state == GameSession::State::playing && displayedSpin > 0.12f)
    {
        g.setColour (juce::Colours::white.withAlpha (0.12f + 0.12f * intensityNormalised));
        for (int i = 0; i < 5; ++i)
        {
            const float y = centre.y - 96.0f + static_cast<float> (i) * 34.0f;
            const float len = 96.0f + intensityNormalised * 146.0f;
            g.drawLine (sceneArea.getX() + 34.0f, y, sceneArea.getX() + 34.0f + len, y, 2.0f);
        }
    }

    if (successState)
    {
        const float bloom = juce::jlimit (0.0f, 1.0f, 0.55f + 0.45f * std::sin (celebrateTime * 2.8f));
        const float rewardY = juce::jmin (infoCard.getY() - 42.0f, towerBaseY + 18.0f);
        const float startX = getWidth() * 0.5f - 150.0f;
        const float spacing = 75.0f;
        static const juce::Colour petalColours[5] = {
            juce::Colour::fromRGB (255, 125, 125),
            juce::Colour::fromRGB (255, 173, 87),
            juce::Colour::fromRGB (255, 218, 92),
            juce::Colour::fromRGB (161, 225, 110),
            juce::Colour::fromRGB (130, 187, 255)
        };

        for (int i = 0; i < 5; ++i)
            drawRewardFlower (g,
                              { startX + spacing * static_cast<float> (i), rewardY },
                              23.0f + 1.5f * std::sin (visualTime * 1.4f + static_cast<float> (i)),
                              bloom,
                              visualTime,
                              petalColours[i],
                              juce::Colour::fromRGB (255, 214, 84));

        drawCelebrationSparkles (g, { startX - 36.0f, rewardY - 44.0f, spacing * 4.0f + 72.0f, 70.0f }, visualTime);
    }

    SceneChrome::StatusCardModel cardModel;
    switch (state)
    {
        case GameSession::State::idle:
            cardModel.badgeText = u8s("PRÊT");
            cardModel.badgeColour = juce::Colour::fromRGBA (255, 173, 104, 216);
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
            cardModel.badgeColour = soundDetected ? juce::Colour::fromRGBA (255, 160, 92, 222)
                                                  : juce::Colour::fromRGBA (255, 255, 255, 52);
            cardModel.headline = soundDetected ? "Gardez le moulin en mouvement"
                                               : "Faites un son stable";
            break;
        case GameSession::State::roundSuccess:
            cardModel.badgeText = u8s("ÉTAPE VALIDÉE");
            cardModel.badgeColour = juce::Colour::fromRGBA (255, 205, 72, 214);
            cardModel.headline = u8s("Moulin lancé");
            break;
        case GameSession::State::programComplete:
            cardModel.badgeText = u8s("TERMINÉ");
            cardModel.badgeColour = juce::Colour::fromRGBA (255, 205, 72, 214);
            cardModel.headline = u8s("Défi du vent terminé");
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
        const int powerSegments = successState ? 5 : juce::jlimit (0, 5, static_cast<int> (std::floor (goalRatio * 5.0f + 0.0001f)));
        const float spinPercent = juce::jlimit (0.0f, 1.0f, displayedSpin / 1.65f) * 100.0f;
        cardModel.progressValue = goalRatio;
        cardModel.progressColour = successState ? juce::Colour::fromRGB (255, 205, 72)
                                                : juce::Colour::fromRGB (255, 157, 92);
        cardModel.footerLeft = "Puissance " + juce::String (powerSegments) + "/5";
        cardModel.footerCentre = "Rotation " + juce::String (spinPercent, 0) + "%";
        cardModel.footerRight = successState ? juce::String ("Bravo")
                                             : (soundDetected ? juce::String ("Continuez")
                                                              : juce::String (u8s("Écoute")));
    }

    SceneChrome::drawStatusCard (g, infoCard, cardModel);

    if (state == GameSession::State::countdown)
        SceneChrome::drawCountdownOverlay (g, getLocalBounds().toFloat(), countdownRemaining,
                                           juce::Colour::fromRGBA (255, 165, 99, 214),
                                           u8s("Préparez le souffle"),
                                           "Commencez avec une voix douce et stable");
}
