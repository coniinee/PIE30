#include "SettingsPanel.h"

namespace
{
    const auto panelFill = juce::Colour::fromRGBA (27, 36, 61, 228);
    const auto panelStroke = juce::Colour::fromRGBA (255, 255, 255, 28);
    const auto cardFill = juce::Colour::fromRGBA (255, 255, 255, 18);
    const auto cardStroke = juce::Colour::fromRGBA (255, 255, 255, 22);

    float dbToNormalised (float db)
    {
        return juce::jmap (juce::jlimit (-80.0f, 0.0f, db), -80.0f, 0.0f, 0.0f, 1.0f);
    }

    void drawSectionCard (juce::Graphics& g,
                          juce::Rectangle<float> area,
                          const juce::String& caption,
                          juce::Colour accent)
    {
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.fillRoundedRectangle (area.translated (0.0f, 4.0f), 18.0f);

        g.setColour (cardFill);
        g.fillRoundedRectangle (area, 18.0f);

        g.setColour (cardStroke);
        g.drawRoundedRectangle (area.reduced (0.5f), 18.0f, 1.0f);

        auto header = area.removeFromTop (22.0f).reduced (12.0f, 0.0f);
        g.setColour (accent);
        g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
        g.drawText (caption.toUpperCase(), header.toNearestInt(), juce::Justification::centredLeft);
    }

    void drawMeterRow (juce::Graphics& g,
                       juce::Rectangle<float> area,
                       const juce::String& label,
                       float valueNormalised,
                       juce::Colour colour)
    {
        auto labelArea = area.removeFromLeft (72.0f);
        g.setColour (juce::Colours::white.withAlpha (0.86f));
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawText (label, labelArea.toNearestInt(), juce::Justification::centredLeft);

        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (area, 7.0f);

        auto fill = area;
        fill.setWidth (area.getWidth() * juce::jlimit (0.0f, 1.0f, valueNormalised));
        g.setColour (colour.withAlpha (0.95f));
        g.fillRoundedRectangle (fill, 7.0f);
    }

    void setLabelTextIfChanged (juce::Label& label, juce::String& cache, const juce::String& newText)
    {
        if (cache != newText)
        {
            cache = newText;
            label.setText (newText, juce::dontSendNotification);
        }
    }

    void setButtonTextIfChanged (juce::TextButton& button, juce::String& cache, const juce::String& newText)
    {
        if (cache != newText)
        {
            cache = newText;
            button.setButtonText (newText);
        }
    }

    class PanelToggleLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawToggleButton (juce::Graphics& g,
                               juce::ToggleButton& button,
                               bool /*shouldDrawButtonAsHighlighted*/,
                               bool /*shouldDrawButtonAsDown*/) override
        {
            auto bounds = button.getLocalBounds().toFloat();

            constexpr float boxSize = 16.0f;
            constexpr float cornerSize = 4.0f;

            auto box = juce::Rectangle<float> (bounds.getX(),
                                               bounds.getCentreY() - boxSize * 0.5f,
                                               boxSize,
                                               boxSize);

            g.setColour (juce::Colour::fromRGBA (255, 255, 255, 22));
            g.fillRoundedRectangle (box, cornerSize);

            g.setColour (juce::Colours::white.withAlpha (0.45f));
            g.drawRoundedRectangle (box, cornerSize, 1.0f);

            if (button.getToggleState())
            {
                auto inner = box.reduced (3.5f);
                g.setColour (juce::Colour::fromRGB (108, 221, 175));
                g.fillRoundedRectangle (inner, 2.5f);

                juce::Path tick;
                tick.startNewSubPath (box.getX() + 4.0f, box.getCentreY());
                tick.lineTo (box.getX() + 7.0f, box.getBottom() - 4.5f);
                tick.lineTo (box.getRight() - 3.8f, box.getY() + 4.2f);

                g.setColour (juce::Colours::white.withAlpha (0.95f));
                g.strokePath (tick, juce::PathStrokeType (1.8f,
                                                          juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
            }

            auto textArea = bounds.withTrimmedLeft (boxSize + 10.0f);

            g.setColour (juce::Colours::white.withAlpha (button.isEnabled() ? 0.88f : 0.45f));
            g.setFont (juce::FontOptions (13.2f));
            g.drawFittedText (button.getButtonText(),
                              textArea.toNearestInt(),
                              juce::Justification::centredLeft,
                              1);
        }
    };

    PanelToggleLookAndFeel& getPanelToggleLookAndFeel()
    {
        static PanelToggleLookAndFeel instance;
        return instance;
    }

    void configureCombo (juce::ComboBox& box)
    {
        box.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGBA (18, 31, 46, 210));
        box.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGBA (255, 255, 255, 42));
        box.setColour (juce::ComboBox::textColourId, juce::Colours::white.withAlpha (0.95f));
        box.setColour (juce::ComboBox::arrowColourId, juce::Colours::white.withAlpha (0.85f));
    }

    void configureCaption (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centredLeft);
        label.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.80f));
        label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    }
}

SettingsPanel::SettingsPanel()
{
    addAndMakeVisible (titleLabel);
    titleLabel.setText (u8s("Panneau de contrôle thérapeute"), juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::FontOptions (20.5f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible (stageLabel);
    stageLabel.setJustificationType (juce::Justification::centredLeft);
    stageLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.96f));
    stageLabel.setFont (juce::FontOptions (12.8f, juce::Font::bold));
    stageLabel.setText (u8s("Guirlande du sapin | Programme guidé | Standard | Étape 1/3 | Étape 1 - Échauffement"), juce::dontSendNotification);

    addAndMakeVisible (statusLabel);
    statusLabel.setJustificationType (juce::Justification::topLeft);
    statusLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.84f));
    statusLabel.setFont (juce::FontOptions (12.6f));
    statusLabel.setText (u8s("Choisissez un jeu, puis appuyez sur Démarrer. Une calibration est effectuée avant chaque tour."),
                         juce::dontSendNotification);

    addAndMakeVisible (gameLabel);
    configureCaption (gameLabel, "Jeu");

    addAndMakeVisible (gameBox);
    gameBox.addItem ("Guirlande du sapin", 1);
    gameBox.addItem (u8s("Moulin à vent"), 2);
    gameBox.addItem ("Vol du papillon", 3);
    gameBox.setSelectedId (1, juce::dontSendNotification);
    configureCombo (gameBox);
    gameBox.onChange = [this]
    {
        if (onGameChanged)
            onGameChanged (gameBox.getSelectedId());
    };

    gameLabel.setVisible (false);
    gameBox.setVisible (false);

    addAndMakeVisible (modeLabel);
    configureCaption (modeLabel, "Mode");

    addAndMakeVisible (modeBox);
    modeBox.addItem (u8s("Entraînement"), 1);
    modeBox.addItem (u8s("Programme guidé"), 2);
    modeBox.setSelectedId (2, juce::dontSendNotification);
    configureCombo (modeBox);
    modeBox.onChange = [this]
    {
        if (onModeChanged)
            onModeChanged (modeBox.getSelectedId());
    };

    addAndMakeVisible (difficultyLabel);
    configureCaption (difficultyLabel, u8s("Difficulté"));

    addAndMakeVisible (difficultyBox);
    difficultyBox.addItem ("Facile", 1);
    difficultyBox.addItem ("Standard", 2);
    difficultyBox.addItem (u8s("Avancé"), 3);
    difficultyBox.setSelectedId (2, juce::dontSendNotification);
    configureCombo (difficultyBox);
    difficultyBox.onChange = [this]
    {
        if (onDifficultyChanged)
            onDifficultyChanged (difficultyBox.getSelectedId());
    };

    addAndMakeVisible (countdownToggle);
    countdownToggle.setButtonText (u8s("Compte à rebours de 3 s après la calibration"));
    countdownToggle.setLookAndFeel (&getPanelToggleLookAndFeel());
    countdownToggle.setClickingTogglesState (true);
    countdownToggle.setToggleState (true, juce::dontSendNotification);
    countdownToggle.onClick = [this]
    {
        if (onCountdownToggled)
            onCountdownToggled (countdownToggle.getToggleState());
    };

    addAndMakeVisible (meterLabel);
    meterLabel.setJustificationType (juce::Justification::topLeft);
    meterLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.84f));
    meterLabel.setFont (juce::FontOptions (12.5f));

    addAndMakeVisible (thresholdLabel);
    thresholdLabel.setJustificationType (juce::Justification::topLeft);
    thresholdLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.84f));
    thresholdLabel.setFont (juce::FontOptions (12.5f));

    addAndMakeVisible (sensitivityLabel);
    sensitivityLabel.setText (u8s("Décalage du seuil (dB, plus bas = plus sensible)"), juce::dontSendNotification);
    sensitivityLabel.setJustificationType (juce::Justification::centredLeft);
    sensitivityLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.82f));
    sensitivityLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));

    addAndMakeVisible (sensitivitySlider);
    sensitivitySlider.setSliderStyle (juce::Slider::LinearHorizontal);
    sensitivitySlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 22);
    sensitivitySlider.setRange (-12.0, 12.0, 0.5);
    sensitivitySlider.setValue (0.0);
    sensitivitySlider.setColour (juce::Slider::trackColourId, juce::Colour::fromRGBA (88, 205, 255, 180));
    sensitivitySlider.setColour (juce::Slider::backgroundColourId, juce::Colours::white.withAlpha (0.08f));
    sensitivitySlider.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB (130, 226, 255));
    sensitivitySlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour::fromRGBA (255, 255, 255, 30));
    sensitivitySlider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    sensitivitySlider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGBA (17, 26, 42, 220));
    sensitivitySlider.onValueChange = [this]
    {
        if (onSensitivityChanged)
            onSensitivityChanged (static_cast<float> (sensitivitySlider.getValue()));
    };

    addAndMakeVisible (statsLabel);
    statsLabel.setJustificationType (juce::Justification::topLeft);
    statsLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.92f));
    statsLabel.setFont (juce::FontOptions (12.4f));
    statsLabel.setText (u8s("Série actuelle : 0.0 s\nMeilleure série : 0.0 s\nTemps vocalisé : 0.0 s\nStabilité : 0 %\nContrôle d’intensité : 0 %"),
                        juce::dontSendNotification);

    addAndMakeVisible (startButton);
    startButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (86, 188, 129));
    startButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour::fromRGB (72, 173, 116));
    startButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    startButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    startButton.onClick = [this]
    {
        if (onStartClicked)
            onStartClicked();
    };

    addAndMakeVisible (resetButton);
    resetButton.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGBA (255, 255, 255, 16));
    resetButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour::fromRGBA (255, 255, 255, 24));
    resetButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.86f));
    resetButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white.withAlpha (0.94f));
    resetButton.onClick = [this]
    {
        if (onResetClicked)
            onResetClicked();
    };
}

void SettingsPanel::updateReadouts (float newRawDb,
                                    float newSmoothDb,
                                    float newNoiseFloorDb,
                                    float newOpenThresholdDb,
                                    float newCloseThresholdDb,
                                    float newIntensityNormalised,
                                    float newPitchHz,
                                    float newPitchNormalised,
                                    bool newSoundDetected,
                                    const juce::String& newStatusText,
                                    const juce::String& newStageText,
                                    const juce::String& newStatsText,
                                    int newTotalStars,
                                    int newLastStars,
                                    const juce::String& primaryActionText,
                                    const juce::String& newControlLabel,
                                    float newControlNormalised)
{
    rawDb = newRawDb;
    smoothDb = newSmoothDb;
    noiseFloorDb = newNoiseFloorDb;
    openThresholdDb = newOpenThresholdDb;
    closeThresholdDb = newCloseThresholdDb;
    intensityNormalised = newIntensityNormalised;
    pitchHz = newPitchHz;
    pitchNormalised = newPitchNormalised;
    controlRowLabel = newControlLabel;
    controlNormalised = newControlNormalised;
    soundDetected = newSoundDetected;
    totalStars = newTotalStars;
    lastStars = newLastStars;

    setLabelTextIfChanged (stageLabel, cachedStageText, newStageText);
    setLabelTextIfChanged (statusLabel, cachedStatusText, newStatusText);
    setLabelTextIfChanged (statsLabel, cachedStatsText, newStatsText);
    setButtonTextIfChanged (startButton, cachedPrimaryActionText, primaryActionText);

    juce::String newMeterText;
    newMeterText << u8s("Niveau brut : ") << juce::String (rawDb, 1) << " dBFS\n"
                 << u8s("Niveau lissé : ") << juce::String (smoothDb, 1) << " dBFS\n"
                 << u8s("Contrôle d’intensité : ") << juce::String (intensityNormalised * 100.0f, 0) << "%";
    newMeterText << u8s("\nHauteur estimée : ");
    if (pitchHz > 0.0f)
        newMeterText << juce::String (pitchHz, 0) << " Hz";
    else
        newMeterText << "--";

    juce::String newThresholdText;
    newThresholdText << u8s("Bruit de fond : ") << juce::String (noiseFloorDb, 1) << " dBFS\n"
                     << u8s("Seuil d’ouverture : ") << juce::String (openThresholdDb, 1) << " dBFS\n"
                     << u8s("Seuil de fermeture : ") << juce::String (closeThresholdDb, 1) << " dBFS";

    setLabelTextIfChanged (meterLabel, cachedMeterText, newMeterText);
    setLabelTextIfChanged (thresholdLabel, cachedThresholdText, newThresholdText);

    repaint();
}

void SettingsPanel::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    juce::ColourGradient panelGradient (juce::Colour::fromRGBA (18, 30, 52, 238), area.getCentreX(), area.getY(),
                                        juce::Colour::fromRGBA (27, 40, 68, 232), area.getCentreX(), area.getBottom(), false);
    panelGradient.addColour (0.65, panelFill);
    g.setGradientFill (panelGradient);
    g.fillRoundedRectangle (area, 24.0f);

    g.setColour (panelStroke);
    g.drawRoundedRectangle (area.reduced (0.5f), 24.0f, 1.0f);

    drawSectionCard (g, headerCardArea.toFloat(),   "Session",           juce::Colour::fromRGB (132, 205, 255));
    drawSectionCard (g, controlCardArea.toFloat(),  "Commandes",          juce::Colour::fromRGB (108, 221, 175));
    drawSectionCard (g, audioCardArea.toFloat(),    "Diagnostic audio", juce::Colour::fromRGB (255, 208, 112));
    drawSectionCard (g, metricsCardArea.toFloat(),  "Mesures du tour",     juce::Colour::fromRGB (215, 180, 255));
    drawSectionCard (g, actionsCardArea.toFloat(),  "Actions",           juce::Colour::fromRGB (132, 205, 255));

    auto stageChip = stageLabel.getBounds().toFloat().reduced (0.0f, 1.0f);
    g.setColour (juce::Colour::fromRGBA (89, 153, 222, 72));
    g.fillRoundedRectangle (stageChip, stageChip.getHeight() * 0.5f);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 18));
    g.drawRoundedRectangle (stageChip.reduced (0.5f), stageChip.getHeight() * 0.5f, 1.0f);

    auto meterBounds = meterArea.toFloat();
    drawMeterRow (g, meterBounds.removeFromTop (18.0f), "Brut", dbToNormalised (rawDb), juce::Colour::fromRGB (86, 181, 255));
    meterBounds.removeFromTop (8.0f);
    drawMeterRow (g, meterBounds.removeFromTop (18.0f), u8s("Lissé"), dbToNormalised (smoothDb), juce::Colour::fromRGB (105, 224, 173));
    meterBounds.removeFromTop (8.0f);
    drawMeterRow (g, meterBounds.removeFromTop (18.0f), "Seuil", dbToNormalised (openThresholdDb), juce::Colour::fromRGB (255, 208, 96));
    meterBounds.removeFromTop (8.0f);
    drawMeterRow (g, meterBounds.removeFromTop (18.0f), controlRowLabel, controlNormalised, juce::Colour::fromRGB (255, 142, 94));

    auto chipBounds = chipArea.toFloat();
    auto voiceChip = chipBounds.removeFromLeft (chipBounds.getWidth() * 0.48f).reduced (4.0f, 0.0f);
    chipBounds.removeFromLeft (6.0f);
    auto starChip = chipBounds.reduced (4.0f, 0.0f);

    g.setColour (soundDetected ? juce::Colour::fromRGBA (66, 212, 123, 235)
                               : juce::Colour::fromRGBA (95, 109, 129, 200));
    g.fillRoundedRectangle (voiceChip, voiceChip.getHeight() * 0.5f);
    g.setColour (juce::Colour::fromRGBA (255, 207, 96, 220));
    g.fillRoundedRectangle (starChip, starChip.getHeight() * 0.5f);

    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawRoundedRectangle (voiceChip.reduced (0.5f), voiceChip.getHeight() * 0.5f, 1.0f);
    g.drawRoundedRectangle (starChip.reduced (0.5f), starChip.getHeight() * 0.5f, 1.0f);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (soundDetected ? u8s("Voix détectée") : juce::String ("En attente de voix"),
                voiceChip.toNearestInt(), juce::Justification::centred);

    auto starText = u8s("Étoiles ") + juce::String (totalStars);
    if (lastStars > 0)
        starText << "   |   +" << juce::String (lastStars) << u8s(" gagnées");

    g.drawText (starText, starChip.toNearestInt(), juce::Justification::centred);
}

void SettingsPanel::resized()
{
    auto area = getLocalBounds().reduced (18);

    constexpr int sectionGap = 12;
    constexpr int padX = 14;
    constexpr int padY = 14;

    auto content = area;

    const int totalGap = sectionGap * 4;
    const int usableHeight = juce::jmax (620, content.getHeight() - totalGap);

    int headerH   = juce::jlimit (128, 150, juce::roundToInt (usableHeight * 0.16f));
    int controlsH = juce::jlimit (144, 168, juce::roundToInt (usableHeight * 0.18f));
    int metricsH  = juce::jlimit (120, 142, juce::roundToInt (usableHeight * 0.145f));
    int actionsH  = juce::jlimit (118, 138, juce::roundToInt (usableHeight * 0.145f));

    int audioH = usableHeight - headerH - controlsH - metricsH - actionsH;
    audioH = juce::jlimit (238, 310, audioH);

    const int usedHeight = headerH + controlsH + audioH + metricsH + actionsH;
    const int heightDelta = usableHeight - usedHeight;

    if (heightDelta > 0)
        audioH += heightDelta;
    else if (heightDelta < 0)
        actionsH = juce::jmax (110, actionsH + heightDelta);

    auto takeSection = [&content] (int h)
    {
        return content.removeFromTop (h);
    };

    headerCardArea = takeSection (headerH);
    content.removeFromTop (sectionGap);

    controlCardArea = takeSection (controlsH);
    content.removeFromTop (sectionGap);

    audioCardArea = takeSection (audioH);
    content.removeFromTop (sectionGap);

    metricsCardArea = takeSection (metricsH);
    content.removeFromTop (sectionGap);

    actionsCardArea = content;

    // Header
    {
        auto r = headerCardArea.reduced (padX, padY);
        r.removeFromTop (18);

        titleLabel.setBounds (r.removeFromTop (30));
        r.removeFromTop (7);

        stageLabel.setBounds (r.removeFromTop (24));
        r.removeFromTop (8);

        statusLabel.setBounds (r.removeFromTop (40));
    }

    // Controls
    {
        auto r = controlCardArea.reduced (padX, padY);
        r.removeFromTop (18);

        modeLabel.setBounds (r.removeFromTop (16));
        r.removeFromTop (5);
        modeBox.setBounds (r.removeFromTop (34));
        r.removeFromTop (10);

        difficultyLabel.setBounds (r.removeFromTop (16));
        r.removeFromTop (5);
        difficultyBox.setBounds (r.removeFromTop (34));
        r.removeFromTop (12);

        countdownToggle.setBounds (r.removeFromTop (28));
    }

    // Audio diagnostics
    {
        auto r = audioCardArea.reduced (padX, padY);
        r.removeFromTop (18);

        meterArea = r.removeFromTop (100);
        r.removeFromTop (10);

        meterLabel.setBounds (r.removeFromTop (54));
        r.removeFromTop (8);

        auto sliderBounds = r.removeFromBottom (30);
        r.removeFromBottom (7);
        auto sensitivityLabelBounds = r.removeFromBottom (18);
        r.removeFromBottom (10);

        thresholdLabel.setBounds (r);
        sensitivityLabel.setBounds (sensitivityLabelBounds);
        sensitivitySlider.setBounds (sliderBounds);
    }

    // Metrics
    {
        auto r = metricsCardArea.reduced (padX, padY);
        r.removeFromTop (18);
        statsLabel.setBounds (r.removeFromTop (92));
    }

    // Actions
    {
        auto r = actionsCardArea.reduced (padX, padY);
        r.removeFromTop (18);

        chipArea = r.removeFromTop (36);
        r.removeFromTop (10);

        buttonsArea = r.removeFromTop (42);

        auto buttons = buttonsArea;
        startButton.setBounds (buttons.removeFromLeft (buttons.getWidth() / 2).withTrimmedRight (8));
        resetButton.setBounds (buttons.withTrimmedLeft (8));
    }
}
