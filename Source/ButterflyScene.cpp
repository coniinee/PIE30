#include "ButterflyScene.h"
#include "SceneChrome.h"
#include <cmath>

namespace
{

    juce::Path makeButterflyWing (float width, float height)
    {
        juce::Path wing;
        wing.startNewSubPath (0.0f, 0.0f);
        wing.cubicTo (-width * 0.45f, -height * 0.72f,
                      -width,       -height * 0.42f,
                      -width * 0.82f, 0.0f);
        wing.cubicTo (-width * 0.95f,  height * 0.34f,
                      -width * 0.48f,  height * 0.78f,
                      -0.08f * width,  height * 0.44f);
        wing.closeSubPath();
        return wing;
    }

    juce::Path makeSparkle (juce::Point<float> centre, float radius)
    {
        juce::Path p;
        p.startNewSubPath (centre.x, centre.y - radius);
        p.lineTo (centre.x + radius * 0.22f, centre.y - radius * 0.22f);
        p.lineTo (centre.x + radius, centre.y);
        p.lineTo (centre.x + radius * 0.22f, centre.y + radius * 0.22f);
        p.lineTo (centre.x, centre.y + radius);
        p.lineTo (centre.x - radius * 0.22f, centre.y + radius * 0.22f);
        p.lineTo (centre.x - radius, centre.y);
        p.lineTo (centre.x - radius * 0.22f, centre.y - radius * 0.22f);
        p.closeSubPath();
        return p;
    }

    void drawFlower (juce::Graphics& g, juce::Point<float> centre, float scale, juce::Colour petalColour)
    {
        for (int i = 0; i < 5; ++i)
        {
            const auto angle = static_cast<float> (i) * juce::MathConstants<float>::twoPi / 5.0f;
            const auto px = centre.x + std::cos (angle) * scale * 0.55f;
            const auto py = centre.y + std::sin (angle) * scale * 0.55f;
            g.setColour (petalColour.withAlpha (0.95f));
            g.fillEllipse ({ px - scale * 0.48f, py - scale * 0.30f, scale * 0.96f, scale * 0.60f });
        }

        g.setColour (juce::Colour::fromRGB (255, 214, 88));
        g.fillEllipse ({ centre.x - scale * 0.30f, centre.y - scale * 0.30f, scale * 0.60f, scale * 0.60f });
    }
}

void ButterflyScene::setSceneData (GameSession::State newState,
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
                                   const juce::String& newStatusText)
{
    const auto nowMs = juce::Time::getMillisecondCounterHiRes();
    const auto dt = lastFrameMs > 0.0 ? static_cast<float> ((nowMs - lastFrameMs) / 1000.0) : 1.0f / 30.0f;
    lastFrameMs = nowMs;

    state = newState;
    progress = juce::jlimit (0.0f, 1.0f, newProgress);
    targetProgress = juce::jlimit (0.05f, 0.95f, newTargetProgress);
    soundDetected = newSoundDetected;
    pitchNormalised = juce::jlimit (0.0f, 1.0f, newPitchNormalised);
    pitchHz = juce::jmax (0.0f, newPitchHz);
    celebrateTime = newCelebrateTime;
    calibrationProgress = juce::jlimit (0.0f, 1.0f, newCalibrationProgress);
    countdownRemaining = juce::jmax (0.0f, newCountdownRemaining);
    totalStars = juce::jmax (0, newTotalStars);
    lastStars = juce::jmax (0, newLastStars);
    stageText = newStageText;
    statusText = newStatusText;

    const auto successState = state == GameSession::State::roundSuccess || state == GameSession::State::programComplete;
    const auto flapSpeed = successState ? 12.0f : (soundDetected ? 16.0f : 6.0f);
    wingPhase = std::fmod (wingPhase + dt * flapSpeed, juce::MathConstants<float>::twoPi);
    hoverOffset = 7.0f * std::sin (celebrateTime * 3.2f + wingPhase * 0.5f);
    repaint();
}

void ButterflyScene::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient sky (juce::Colour::fromRGB (109, 191, 255), bounds.getCentreX(), bounds.getBottom(),
                              juce::Colour::fromRGB (242, 249, 255), bounds.getCentreX(), bounds.getY(), false);
    sky.addColour (0.62, juce::Colour::fromRGB (173, 224, 255));
    g.setGradientFill (sky);
    g.fillAll();

    g.setColour (juce::Colour::fromRGBA (255, 234, 161, 180));
    g.fillEllipse ({ bounds.getRight() - 170.0f, 38.0f, 86.0f, 86.0f });

    for (int i = 0; i < 4; ++i)
    {
        const float cloudX = 48.0f + static_cast<float> (i) * 210.0f + 16.0f * std::sin (celebrateTime * 0.7f + static_cast<float> (i));
        const float cloudY = 74.0f + static_cast<float> (i % 2) * 40.0f;
        g.setColour (juce::Colours::white.withAlpha (0.45f));
        g.fillEllipse (cloudX, cloudY, 110.0f, 38.0f);
        g.fillEllipse (cloudX + 24.0f, cloudY - 12.0f, 66.0f, 52.0f);
        g.fillEllipse (cloudX + 56.0f, cloudY + 3.0f, 86.0f, 34.0f);
    }

    auto meadow = bounds.removeFromBottom (bounds.getHeight() * 0.25f);
    g.setColour (juce::Colour::fromRGB (105, 188, 103));
    g.fillEllipse ({ -20.0f, meadow.getY() - 18.0f, meadow.getWidth() * 0.64f, meadow.getHeight() * 1.75f });
    g.fillEllipse ({ meadow.getWidth() * 0.25f, meadow.getY() - 28.0f, meadow.getWidth() * 0.90f, meadow.getHeight() * 1.85f });

    drawFlower (g, { 112.0f, meadow.getY() + 42.0f }, 22.0f, juce::Colour::fromRGB (255, 128, 168));
    drawFlower (g, { 170.0f, meadow.getY() + 58.0f }, 18.0f, juce::Colour::fromRGB (182, 144, 255));
    drawFlower (g, { getWidth() - 150.0f, meadow.getY() + 48.0f }, 24.0f, juce::Colour::fromRGB (255, 148, 120));
    drawFlower (g, { getWidth() - 90.0f, meadow.getY() + 62.0f }, 18.0f, juce::Colour::fromRGB (255, 214, 112));

    SceneChrome::drawTopHud (g, getLocalBounds().toFloat(), "Vol du papillon", stageText, totalStars, lastStars);

    auto flightArea = getLocalBounds().toFloat().reduced (56.0f, 64.0f);
    flightArea.removeFromTop (82.0f);
    flightArea.removeFromBottom (88.0f);
    flightArea.removeFromLeft (16.0f);
    flightArea.removeFromRight (16.0f);

    const auto targetY = juce::jmap (targetProgress, 0.0f, 1.0f, flightArea.getBottom() - 28.0f, flightArea.getY() + 28.0f);
    const auto butterflyY = juce::jmap (progress, 0.0f, 1.0f, flightArea.getBottom() - 34.0f, flightArea.getY() + 34.0f) + hoverOffset;
    const auto butterflyX = flightArea.getCentreX() + 18.0f * std::sin (wingPhase * 0.35f + celebrateTime * 0.7f);

    auto targetBand = juce::Rectangle<float> (flightArea.getX() + 68.0f, targetY - 16.0f, flightArea.getWidth() - 136.0f, 32.0f);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 38));
    g.fillRoundedRectangle (targetBand, 16.0f);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 72));
    g.drawRoundedRectangle (targetBand.reduced (0.5f), 16.0f, 1.1f);

    drawFlower (g, { targetBand.getX() + 12.0f, targetY }, 18.0f, juce::Colour::fromRGB (255, 136, 171));
    drawFlower (g, { targetBand.getRight() - 12.0f, targetY }, 18.0f, juce::Colour::fromRGB (142, 183, 255));

    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.drawLine (flightArea.getCentreX(), flightArea.getY() + 8.0f,
                flightArea.getCentreX(), flightArea.getBottom() - 10.0f,
                1.2f);

    const auto successState = state == GameSession::State::roundSuccess || state == GameSession::State::programComplete;
    const auto wingSpread = successState ? 1.30f : (0.80f + 0.32f * std::sin (wingPhase));
    const auto bodyCentre = juce::Point<float> (butterflyX, butterflyY);

    auto leftWing = makeButterflyWing (54.0f * wingSpread, 70.0f);
    leftWing.applyTransform (juce::AffineTransform::translation (bodyCentre.x - 8.0f, bodyCentre.y));

    auto rightWing = makeButterflyWing (54.0f * wingSpread, 70.0f);
    rightWing.applyTransform (juce::AffineTransform::scale (-1.0f, 1.0f)
                                  .translated (bodyCentre.x + 8.0f, bodyCentre.y));

    g.setColour (juce::Colour::fromRGBA (255, 156, 196, 84));
    g.fillEllipse ({ bodyCentre.x - 84.0f, bodyCentre.y - 74.0f, 168.0f, 148.0f });

    g.setColour (juce::Colour::fromRGB (255, 140, 186));
    g.fillPath (leftWing);
    g.setColour (juce::Colour::fromRGB (147, 203, 255));
    g.fillPath (rightWing);

    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 110));
    g.strokePath (leftWing, juce::PathStrokeType (1.6f));
    g.strokePath (rightWing, juce::PathStrokeType (1.6f));

    g.setColour (juce::Colour::fromRGB (88, 74, 118));
    g.fillRoundedRectangle ({ bodyCentre.x - 6.0f, bodyCentre.y - 28.0f, 12.0f, 56.0f }, 6.0f);
    g.fillEllipse ({ bodyCentre.x - 7.5f, bodyCentre.y - 40.0f, 15.0f, 15.0f });

    juce::Path antennae;
    antennae.startNewSubPath (bodyCentre.x - 2.0f, bodyCentre.y - 32.0f);
    antennae.quadraticTo (bodyCentre.x - 12.0f, bodyCentre.y - 54.0f, bodyCentre.x - 18.0f, bodyCentre.y - 60.0f);
    antennae.startNewSubPath (bodyCentre.x + 2.0f, bodyCentre.y - 32.0f);
    antennae.quadraticTo (bodyCentre.x + 12.0f, bodyCentre.y - 54.0f, bodyCentre.x + 18.0f, bodyCentre.y - 60.0f);
    g.strokePath (antennae, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.fillEllipse ({ bodyCentre.x - 20.5f, bodyCentre.y - 62.5f, 5.0f, 5.0f });
    g.fillEllipse ({ bodyCentre.x + 15.5f, bodyCentre.y - 62.5f, 5.0f, 5.0f });

    if (soundDetected)
    {
        const auto trailAlpha = 0.18f + 0.16f * pitchNormalised;
        for (int i = 1; i <= 4; ++i)
        {
            g.setColour (juce::Colours::white.withAlpha (trailAlpha / static_cast<float> (i)));
            g.fillEllipse ({ bodyCentre.x - 18.0f * static_cast<float> (i), bodyCentre.y + 4.0f * static_cast<float> (i) - 7.0f,
                             14.0f - static_cast<float> (i) * 2.0f, 14.0f - static_cast<float> (i) * 2.0f });
        }
    }

    if (successState)
    {
        for (int i = 0; i < 6; ++i)
        {
            const auto angle = celebrateTime * 1.6f + static_cast<float> (i) * 0.9f;
            const auto centre = juce::Point<float> (bodyCentre.x + std::cos (angle) * 72.0f,
                                                    bodyCentre.y + std::sin (angle * 1.15f) * 58.0f);
            g.setColour (juce::Colours::white.withAlpha (0.70f));
            g.fillPath (makeSparkle (centre, 8.0f));
        }
    }

    SceneChrome::StatusCardModel cardModel;
    switch (state)
    {
        case GameSession::State::idle:
            cardModel.badgeText = u8s("PRÊT");
            cardModel.badgeColour = juce::Colour::fromRGBA (255, 157, 196, 216);
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
            cardModel.badgeColour = soundDetected ? juce::Colour::fromRGBA (255, 144, 188, 220)
                                                  : juce::Colour::fromRGBA (255, 255, 255, 52);
            cardModel.headline = soundDetected ? "Utilisez la hauteur pour guider le papillon"
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
        cardModel.progressValue = progress;
        cardModel.progressColour = successState ? juce::Colour::fromRGB (255, 205, 72)
                                                : juce::Colour::fromRGB (255, 144, 188);
        cardModel.footerLeft = "Altitude " + juce::String (progress * 100.0f, 0) + "%";
        cardModel.footerCentre = pitchHz > 0.0f ? (juce::String ("Hauteur ") + juce::String (pitchHz, 0) + " Hz")
                                                : juce::String ("Hauteur --");
        cardModel.footerRight = successState ? juce::String ("Bravo")
                                             : ((soundDetected && pitchHz > 0.0f) ? juce::String ("Suivi")
                                                                                  : (soundDetected ? juce::String ("Voix active")
                                                                                                   : juce::String (u8s("Écoute"))));
    }

    const auto cardWidth = juce::jlimit (430.0f, 560.0f, getWidth() * 0.50f);
    auto card = juce::Rectangle<float> (flightArea.getCentreX() - cardWidth * 0.5f, getHeight() - 138.0f, cardWidth, 118.0f);
    SceneChrome::drawStatusCard (g, card, cardModel);

    if (state == GameSession::State::countdown)
        SceneChrome::drawCountdownOverlay (g, getLocalBounds().toFloat(), countdownRemaining,
                                           juce::Colour::fromRGBA (255, 157, 196, 214),
                                           u8s("Préparez-vous à voler"),
                                           "Commencez par un son stable");
}
