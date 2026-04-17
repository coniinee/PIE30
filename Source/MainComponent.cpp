#include "MainComponent.h"
#include <cmath>

namespace
{
    GameSession::GameType gameTypeFromId (int id) noexcept
    {
        switch (id)
        {
            case 2:  return GameSession::GameType::windmill;
            case 3:  return GameSession::GameType::butterflyPitch;
            default: return GameSession::GameType::treeLights;
        }
    }

    GameSession::Mode modeFromId (int id) noexcept
    {
        return id == 1 ? GameSession::Mode::practice : GameSession::Mode::guidedProgram;
    }

    GameSession::Difficulty difficultyFromId (int id) noexcept
    {
        switch (id)
        {
            case 1:  return GameSession::Difficulty::easy;
            case 3:  return GameSession::Difficulty::advanced;
            default: return GameSession::Difficulty::standard;
        }
    }

    juce::String compactGoalTitle (juce::String goalTitle)
    {
        const auto separator = goalTitle.indexOf (" - ");
        if (separator >= 0)
            return goalTitle.fromFirstOccurrenceOf (" - ", false, false).trim();

        return goalTitle.trim();
    }

    juce::String buildPanelStageText (const GameSession& session)
    {
        juce::String text;
        text << session.getGameTypeName() << " | "
             << session.getModeName() << " | "
             << session.getDifficultyName();

        if (session.getMode() == GameSession::Mode::guidedProgram)
            text << u8s(" | Étape ") << juce::String (session.getCurrentStageIndex() + 1)
                 << "/" << juce::String (session.getStageCount());

        text << " | " << session.getCurrentGoal().title;
        return text;
    }

    juce::String buildSceneStageText (const GameSession& session)
    {
        juce::String text;
        text << session.getGameTypeName() << " | "
             << (session.getMode() == GameSession::Mode::practice ? u8s("Entraînement") : u8s("Guidé"))
             << " | " << session.getDifficultyName();

        if (session.getMode() == GameSession::Mode::guidedProgram)
            text << " | " << juce::String (session.getCurrentStageIndex() + 1)
                 << "/" << juce::String (session.getStageCount())
                 << " | " << compactGoalTitle (session.getCurrentGoal().title);

        return text;
    }

    void configureSelectionButton (juce::TextButton& button)
    {
        button.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGBA (255, 255, 255, 18));
        button.setColour (juce::TextButton::buttonOnColourId, juce::Colour::fromRGBA (255, 255, 255, 28));
        button.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    }

    void updateSelectionButtonStyle (juce::TextButton& button, bool isSelected)
    {
        button.setColour (juce::TextButton::buttonColourId,
                          isSelected ? juce::Colour::fromRGBA (78, 205, 142, 224)
                                     : juce::Colour::fromRGBA (255, 255, 255, 22));
        button.setColour (juce::TextButton::buttonOnColourId,
                          isSelected ? juce::Colour::fromRGBA (66, 186, 123, 235)
                                     : juce::Colour::fromRGBA (255, 255, 255, 30));
        button.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
        button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    }
}

MainComponent::MainComponent()
    : analyzer(),
      session(),
      treeScene(),
      windmillScene(),
      butterflyScene(),
      settingsPanel()
{
    setOpaque (true);

    addAndMakeVisible (treeScene);
    addAndMakeVisible (windmillScene);
    addAndMakeVisible (butterflyScene);
    addAndMakeVisible (settingsPanel);
    addAndMakeVisible (gameSelectionOverlay);

    gameSelectionOverlay.setInterceptsMouseClicks (false, true);
    gameSelectionOverlay.addAndMakeVisible (gameSelectionTitleLabel);
    gameSelectionOverlay.addAndMakeVisible (gameSelectionSubtitleLabel);
    gameSelectionOverlay.addAndMakeVisible (treeGameButton);
    gameSelectionOverlay.addAndMakeVisible (windmillGameButton);
    gameSelectionOverlay.addAndMakeVisible (butterflyGameButton);

    gameSelectionTitleLabel.setText ("Choisissez un jeu", juce::dontSendNotification);
    gameSelectionTitleLabel.setJustificationType (juce::Justification::centred);
    gameSelectionTitleLabel.setFont (juce::FontOptions (31.0f, juce::Font::bold));
    gameSelectionTitleLabel.setColour (juce::Label::textColourId, juce::Colours::white);

    gameSelectionSubtitleLabel.setText (u8s("Sélectionnez une activité ici, puis appuyez sur Démarrer dans le panneau de droite."), juce::dontSendNotification);
    gameSelectionSubtitleLabel.setJustificationType (juce::Justification::centred);
    gameSelectionSubtitleLabel.setFont (juce::FontOptions (15.0f));
    gameSelectionSubtitleLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.82f));

    configureSelectionButton (treeGameButton);
    configureSelectionButton (windmillGameButton);
    configureSelectionButton (butterflyGameButton);

    treeGameButton.onClick = [this]
    {
        session.setGameType (GameSession::GameType::treeLights);
        analyzer.reset();
        refreshUi();
    };

    windmillGameButton.onClick = [this]
    {
        session.setGameType (GameSession::GameType::windmill);
        analyzer.reset();
        refreshUi();
    };

    butterflyGameButton.onClick = [this]
    {
        session.setGameType (GameSession::GameType::butterflyPitch);
        analyzer.reset();
        refreshUi();
    };

    settingsPanel.onStartClicked = [this] { startSession(); };
    settingsPanel.onResetClicked = [this] { resetSession(); };
    settingsPanel.onSensitivityChanged = [this] (float value)
    {
        analyzer.setSensitivityOffsetDb (value);
        refreshUi();
    };

    settingsPanel.onGameChanged = [this] (int selectedId)
    {
        session.setGameType (gameTypeFromId (selectedId));
        analyzer.reset();
        refreshUi();
    };

    settingsPanel.onModeChanged = [this] (int selectedId)
    {
        session.setMode (modeFromId (selectedId));
        analyzer.reset();
        refreshUi();
    };

    settingsPanel.onDifficultyChanged = [this] (int selectedId)
    {
        session.setDifficulty (difficultyFromId (selectedId));
        analyzer.reset();
        refreshUi();
    };

    settingsPanel.onCountdownToggled = [this] (bool enabled)
    {
        session.setCountdownEnabled (enabled);
        refreshUi();
    };

    initialiseAudio();

    setSize (1400, 1000);
    startTimerHz (30);
    refreshUi();
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (9, 18, 36));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (18);

    const int gap = 14;
    const int rightWidth = juce::jlimit (360, 430, juce::roundToInt (getWidth() * 0.30f));

    auto rightPanel = area.removeFromRight (rightWidth);
    area.removeFromRight (gap);

    treeScene.setBounds (area);
    windmillScene.setBounds (area);
    butterflyScene.setBounds (area);
    settingsPanel.setBounds (rightPanel);

    gameSelectionOverlay.setBounds (area);

    const int cardWidth = juce::jlimit (430, 620, juce::roundToInt (area.getWidth() * 0.52f));
    const int cardHeight = juce::jlimit (300, 400, juce::roundToInt (area.getHeight() * 0.40f));
    auto cardBounds = juce::Rectangle<int> (cardWidth, cardHeight).withCentre (area.getCentre());
    gameSelectionOverlay.setCardBounds (gameSelectionOverlay.getLocalArea (this, cardBounds));

    auto content = gameSelectionOverlay.getCardBounds().reduced (32, 24);
    gameSelectionTitleLabel.setBounds (content.removeFromTop (44));
    content.removeFromTop (10);
    gameSelectionSubtitleLabel.setBounds (content.removeFromTop (42));
    content.removeFromTop (20);

    const int buttonHeight = 54;
    treeGameButton.setBounds (content.removeFromTop (buttonHeight));
    content.removeFromTop (12);
    windmillGameButton.setBounds (content.removeFromTop (buttonHeight));
    content.removeFromTop (12);
    butterflyGameButton.setBounds (content.removeFromTop (buttonHeight));
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    analyzer.prepare (sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (! audioReady || bufferToFill.buffer == nullptr)
        return;

    int activeInputChannels = 0;

    if (auto* device = deviceManager.getCurrentAudioDevice())
        activeInputChannels = device->getActiveInputChannels().countNumberOfSetBits();

    analyzer.processBlock (*bufferToFill.buffer,
                           bufferToFill.startSample,
                           bufferToFill.numSamples,
                           activeInputChannels);
}

void MainComponent::releaseResources()
{
}

void MainComponent::timerCallback()
{
    const auto nowMs = juce::Time::getMillisecondCounterHiRes();
    const auto dt = lastUiTickMs > 0.0 ? static_cast<float> ((nowMs - lastUiTickMs) / 1000.0)
                                       : 1.0f / 30.0f;
    lastUiTickMs = nowMs;

    session.update (dt,
                    analyzer.isSoundDetected(),
                    analyzer.isCalibrating(),
                    analyzer.getIntensityNormalised(),
                    analyzer.getPitchNormalised());
    refreshUi();
}

void MainComponent::initialiseAudio()
{
   #if JUCE_ANDROID || JUCE_IOS
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [this] (bool granted)
                                           {
                                               juce::MessageManager::callAsync ([this, granted]
                                               {
                                                   if (granted)
                                                   {
                                                       permissionDenied = false;
                                                       setAudioChannels (1, 0);
                                                       audioReady = true;
                                                   }
                                                   else
                                                   {
                                                       permissionDenied = true;
                                                       audioReady = false;
                                                   }

                                                   refreshUi();
                                               });
                                           });
        return;
    }
   #endif

    setAudioChannels (1, 0);
    audioReady = true;
}

void MainComponent::startSession()
{
    if (! audioReady)
        return;

    permissionDenied = false;
    analyzer.startCalibration (1.5);
    session.startCalibration();
    lastUiTickMs = juce::Time::getMillisecondCounterHiRes();
    refreshUi();
}

void MainComponent::resetSession()
{
    analyzer.reset();
    session.reset();
    refreshUi();
}

void MainComponent::updateSceneVisibility()
{
    const auto gameType = session.getGameType();
    treeScene.setVisible (gameType == GameSession::GameType::treeLights);
    windmillScene.setVisible (gameType == GameSession::GameType::windmill);
    butterflyScene.setVisible (gameType == GameSession::GameType::butterflyPitch);
}

void MainComponent::updateGameSelectionOverlay()
{
    const auto showOverlay = session.getState() == GameSession::State::idle;
    gameSelectionOverlay.setVisible (showOverlay);

    if (! showOverlay)
        return;

    const auto gameType = session.getGameType();
    updateSelectionButtonStyle (treeGameButton, gameType == GameSession::GameType::treeLights);
    updateSelectionButtonStyle (windmillGameButton, gameType == GameSession::GameType::windmill);
    updateSelectionButtonStyle (butterflyGameButton, gameType == GameSession::GameType::butterflyPitch);
}

void MainComponent::refreshUi()
{
    updateSceneVisibility();
    updateGameSelectionOverlay();

    juce::String statusText;
    const auto gameType = session.getGameType();
    const auto isWindmill = gameType == GameSession::GameType::windmill;
    const auto isButterfly = gameType == GameSession::GameType::butterflyPitch;

    if (permissionDenied)
    {
        statusText = u8s("Accès au microphone refusé. Activez-le dans les réglages du système puis relancez l’application.");
    }
    else if (! audioReady)
    {
        statusText = u8s("L’entrée audio n’est pas encore prête.");
    }
    else
    {
        switch (session.getState())
        {
            case GameSession::State::idle:
                if (isWindmill)
                    statusText = u8s("Moulin à vent sélectionné. Appuyez sur Démarrer dans le panneau de droite. Après la calibration, une voix plus forte fait tourner les ailes plus vite.");
                else if (isButterfly)
                    statusText = u8s("Vol du papillon sélectionné. Appuyez sur Démarrer, puis utilisez une hauteur plus grave ou plus aiguë pour faire descendre ou monter le papillon.");
                else if (session.getMode() == GameSession::Mode::practice)
                    statusText = u8s("Guirlande du sapin sélectionnée. Appuyez sur Démarrer pour lancer un tour d’entraînement. La calibration se fait d’abord.");
                else
                    statusText = u8s("Guirlande du sapin sélectionnée. Appuyez sur Démarrer pour lancer le programme guidé en 3 étapes. Chaque tour recalibre le micro.");
                break;

            case GameSession::State::calibrating:
                statusText = u8s("Mesure du bruit de fond. Gardez la pièce calme pendant environ 1,5 seconde.");
                break;

            case GameSession::State::countdown:
                statusText = u8s("Calibration terminée. Préparez-vous : l’activité commence dans ")
                             + juce::String (static_cast<int> (std::ceil (session.getCountdownRemaining()))) + "...";
                break;

            case GameSession::State::playing:
                if (analyzer.isSoundDetected())
                    statusText = session.getCurrentGoal().instruction;
                else if (isWindmill)
                    statusText = u8s("Aucune voix stable détectée pour le moment. Produisez un son tenu pour démarrer le moulin, puis augmentez l’intensité pour le faire tourner plus vite.");
                else if (isButterfly)
                    statusText = u8s("Aucune voix stable détectée pour le moment. Produisez un son tenu, puis changez la hauteur pour faire monter ou descendre le papillon.");
                else
                    statusText = u8s("Aucune voix stable détectée pour le moment. Produisez un son tenu pour faire monter les lumières.");
                break;

            case GameSession::State::roundSuccess:
                statusText = session.getMode() == GameSession::Mode::guidedProgram
                               ? u8s("Tour validé. Appuyez sur Démarrer pour recalibrer et passer à l’étape suivante.")
                               : u8s("Tour d’entraînement validé. Appuyez sur Démarrer pour rejouer ou sur Réinitialiser pour revenir à l’état initial.");
                break;

            case GameSession::State::programComplete:
                statusText = u8s("Toutes les étapes sont validées. Appuyez sur Démarrer pour relancer le programme guidé complet.");
                break;
        }
    }

    const auto panelStageText = buildPanelStageText (session);
    const auto sceneStageText = buildSceneStageText (session);
    const auto intensity = analyzer.getIntensityNormalised();
    const auto pitchHz = analyzer.getPitchHz();
    const auto pitchNormalised = analyzer.getPitchNormalised();

    juce::String statsText;
    statsText << u8s("Série actuelle : ") << juce::String (session.getCurrentStreak(), 1) << " s\n"
              << u8s("Meilleure série : ") << juce::String (session.getLongestStreak(), 1) << " s\n"
              << u8s("Temps vocalisé : ") << juce::String (session.getRoundVoicedTime(), 1) << " s\n"
              << u8s("Stabilité : ") << juce::String (session.getRoundStability() * 100.0f, 0) << "%\n";

    if (isButterfly)
    {
        statsText << u8s("Hauteur estimée : ") << (pitchHz > 0.0f ? juce::String (pitchHz, 0) + " Hz" : juce::String ("--")) << "\n"
                  << u8s("Contrôle de hauteur : ") << juce::String (pitchNormalised * 100.0f, 0) << "%";
    }
    else
    {
        statsText << u8s("Contrôle d’intensité : ") << juce::String (intensity * 100.0f, 0) << "%";
    }

    settingsPanel.updateReadouts (analyzer.getRawDb(),
                                  analyzer.getSmoothedDb(),
                                  analyzer.getNoiseFloorDb(),
                                  analyzer.getOpenThresholdDb(),
                                  analyzer.getCloseThresholdDb(),
                                  intensity,
                                  pitchHz,
                                  pitchNormalised,
                                  analyzer.isSoundDetected(),
                                  statusText,
                                  panelStageText,
                                  statsText,
                                  session.getTotalStars(),
                                  session.getLastAwardedStars(),
                                  session.getPrimaryActionText(),
                                  isButterfly ? juce::String ("Hauteur") : juce::String (u8s("Contrôle")),
                                  isButterfly ? pitchNormalised : intensity);

    if (gameType == GameSession::GameType::treeLights)
    {
        treeScene.setSceneData (session.getState(),
                                session.getProgress(),
                                session.getTargetProgress(),
                                analyzer.isSoundDetected(),
                                session.getCelebrateTime(),
                                analyzer.getCalibrationProgress(),
                                session.getCountdownRemaining(),
                                session.getTotalStars(),
                                session.getLastAwardedStars(),
                                sceneStageText,
                                statusText);
    }
    else if (gameType == GameSession::GameType::windmill)
    {
        windmillScene.setSceneData (session.getState(),
                                    session.getProgress(),
                                    session.getTargetProgress(),
                                    analyzer.isSoundDetected(),
                                    intensity,
                                    session.getCelebrateTime(),
                                    analyzer.getCalibrationProgress(),
                                    session.getCountdownRemaining(),
                                    session.getTotalStars(),
                                    session.getLastAwardedStars(),
                                    sceneStageText,
                                    statusText);
    }
    else
    {
        butterflyScene.setSceneData (session.getState(),
                                     session.getProgress(),
                                     session.getTargetProgress(),
                                     analyzer.isSoundDetected(),
                                     pitchNormalised,
                                     pitchHz,
                                     session.getCelebrateTime(),
                                     analyzer.getCalibrationProgress(),
                                     session.getCountdownRemaining(),
                                     session.getTotalStars(),
                                     session.getLastAwardedStars(),
                                     sceneStageText,
                                     statusText);
    }
}
