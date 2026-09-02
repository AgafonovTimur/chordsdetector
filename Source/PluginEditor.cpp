#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
namespace colours
{
    static const juce::Colour panel        { 0xff141414 };
    static const juce::Colour section      { 0xff1a1a1a };
    static const juce::Colour sectionEdge  { 0xff2a2a2a };
    static const juce::Colour label        { 0xffdddddd };
    static const juce::Colour dimLabel     { 0xff999999 };
    static const juce::Colour buttonOff    { 0xff1f1f1f };
    static const juce::Colour buttonOn     { 0xff6b6b6b };
    static const juce::Colour scrollThumb  { 0xff5a5a5a };
}

static const char* TONIC_NAMES[12] =
    { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

//==============================================================================
void SettingsPanel::styleTextButton (juce::TextButton& b, bool on)
{
    b.setColour (juce::TextButton::buttonColourId, on ? colours::buttonOn : colours::buttonOff);
    b.setColour (juce::TextButton::textColourOffId, on ? juce::Colours::white : colours::label);
    b.setColour (juce::TextButton::textColourOnId,  juce::Colours::white);
}

SettingsPanel::SettingsPanel (ChordsDetectorProcessor& p) : processor (p)
{
    auto styleSlider = [this] (juce::Slider& s, double lo, double hi, double step)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 62, 22);
        s.setRange (lo, hi, step);
        s.setColour (juce::Slider::trackColourId, colours::buttonOn);
        s.setColour (juce::Slider::backgroundColourId, colours::buttonOff);
        s.setColour (juce::Slider::thumbColourId, juce::Colours::white.withAlpha (0.85f));
        s.setColour (juce::Slider::textBoxTextColourId, colours::label);
        s.setColour (juce::Slider::textBoxOutlineColourId, colours::sectionEdge);
        s.onValueChange = [this] { changed(); };
        addAndMakeVisible (s);
    };

    auto styleToggle = [this] (juce::ToggleButton& b)
    {
        b.setColour (juce::ToggleButton::textColourId, colours::label);
        b.setColour (juce::ToggleButton::tickColourId, juce::Colours::white);
        b.onClick = [this] { changed(); };
        addAndMakeVisible (b);
    };

    styleToggle (midiThruToggle);
    styleToggle (fitToWindowToggle);
    styleToggle (showInversionToggle);
    styleToggle (showDegreeToggle);
    styleToggle (showPolyToggle);

    styleSlider (captureSlider,   10.0, 500.0, 5.0);
    styleSlider (chordSizeSlider, 10.0, 400.0, 5.0);
    styleSlider (invSizeSlider,   10.0, 400.0, 5.0);
    styleSlider (invXSlider,       0.0, 100.0, 1.0);
    styleSlider (invYSlider,       0.0, 100.0, 1.0);
    styleSlider (degSizeSlider,   10.0, 400.0, 5.0);
    styleSlider (degXSlider,       0.0, 100.0, 1.0);
    styleSlider (degYSlider,       0.0, 100.0, 1.0);
    styleSlider (polyXSlider,      0.0, 100.0, 1.0);
    styleSlider (polyYSlider,      0.0, 100.0, 1.0);

    for (auto* l : { &captureLabel, &chordSizeLabel, &invSizeLabel, &invXLabel, &invYLabel,
                     &degSizeLabel, &degXLabel, &degYLabel, &polyXLabel, &polyYLabel,
                     &bgColourLabel, &textColourLabel })
    {
        l->setColour (juce::Label::textColourId, colours::dimLabel);
        l->setFont (juce::Font (juce::FontOptions (14.0f)));
        addAndMakeVisible (*l);
    }

    // ---- Выбор языка ----
    for (int i = 0; i < 3; ++i)
    {
        auto* b = new juce::TextButton();
        b->onClick = [this, i]
        {
            processor.settings.language = i;
            updateTexts();
            refreshFromSettings();
            resized();
            changed();
        };
        addAndMakeVisible (b);
        languageButtons.add (b);
    }

    // ---- Цвета ----
    bgColourButton.onClick   = [this] { openColourPicker (0, bgColourButton); };
    textColourButton.onClick = [this] { openColourPicker (1, textColourButton); };
    addAndMakeVisible (bgColourButton);
    addAndMakeVisible (textColourButton);

    // ---- Бемоль вместо диеза — отдельно по каждой ноте ----
    for (int idx : ChordEngine::ACCIDENTAL_INDICES)
    {
        auto* b = new juce::ToggleButton (juce::String (ChordEngine::NOTE_NAMES_SHARP[idx])
                                          + "/" + ChordEngine::NOTE_NAMES_FLAT[idx]);
        b->setColour (juce::ToggleButton::textColourId, colours::label);
        b->setColour (juce::ToggleButton::tickColourId, juce::Colours::white);
        b->onClick = [this] { changed(); };
        addAndMakeVisible (b);
        accidentalToggles.add (b);
    }

    // ---- Тоники ----
    for (int i = 0; i < 12; ++i)
    {
        auto* b = new juce::TextButton (TONIC_NAMES[i]);
        b->onClick = [this, i]
        {
            processor.settings.tonicPc = (processor.settings.tonicPc == i) ? -1 : i;
            refreshFromSettings();
            changed();
        };
        addAndMakeVisible (b);
        tonicButtons.add (b);
    }

    // ---- Лады ----
    for (auto& m : ChordEngine::modes())
    {
        const juce::String key (m.key);
        auto* b = new juce::TextButton();
        b->onClick = [this, key]
        {
            processor.settings.modeKey = (processor.settings.modeKey == key) ? juce::String() : key;
            refreshFromSettings();
            changed();
        };
        addAndMakeVisible (b);
        modeButtons.add (b);
    }

    // ---- Сброс настроек ----
    resetButton.onClick = [this]
    {
        const int keepLanguage = processor.settings.language;
        processor.settings = Settings();
        processor.settings.language = keepLanguage;
        updateTexts();
        refreshFromSettings();
        resized();
        changed();
    };
    addAndMakeVisible (resetButton);

    updateTexts();
    refreshFromSettings();
}

//==============================================================================
void SettingsPanel::openColourPicker (int target, juce::Component& anchor)
{
    activeColourTarget = target;

    auto selector = std::make_unique<juce::ColourSelector> (
        juce::ColourSelector::showColourAtTop
        | juce::ColourSelector::showSliders
        | juce::ColourSelector::showColourspace);

    selector->setCurrentColour (juce::Colour (target == 0 ? processor.settings.backgroundColour
                                                          : processor.settings.textColour));
    selector->setSize (280, 320);
    selector->addChangeListener (this);

    juce::CallOutBox::launchAsynchronously (std::move (selector),
                                            anchor.getScreenBounds(),
                                            nullptr);
}

void SettingsPanel::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (auto* cs = dynamic_cast<juce::ColourSelector*> (source))
    {
        const auto argb = cs->getCurrentColour().getARGB();

        if (activeColourTarget == 0)
            processor.settings.backgroundColour = argb;
        else if (activeColourTarget == 1)
            processor.settings.textColour = argb;

        refreshFromSettings();

        if (onSettingsChanged)
            onSettingsChanged();
    }
}

//==============================================================================
void SettingsPanel::updateTexts()
{
    const int l = processor.settings.language;

    for (int i = 0; i < languageButtons.size(); ++i)
        languageButtons[i]->setButtonText (Lang::languageName (i));

    midiThruToggle.setButtonText     (Lang::midiThru (l));
    fitToWindowToggle.setButtonText  (Lang::fitToWindow (l));
    showInversionToggle.setButtonText(Lang::showInversion (l));
    showDegreeToggle.setButtonText   (Lang::showDegree (l));
    showPolyToggle.setButtonText     (Lang::showPoly (l));

    captureLabel.setText    (Lang::captureWindow (l), juce::dontSendNotification);
    chordSizeLabel.setText  (Lang::chordSize (l),     juce::dontSendNotification);
    bgColourLabel.setText   (Lang::bgColour (l),      juce::dontSendNotification);
    textColourLabel.setText (Lang::textColour (l),    juce::dontSendNotification);

    invSizeLabel.setText (Lang::size (l), juce::dontSendNotification);
    invXLabel.setText    (Lang::posX (l), juce::dontSendNotification);
    invYLabel.setText    (Lang::posY (l), juce::dontSendNotification);
    degSizeLabel.setText (Lang::size (l), juce::dontSendNotification);
    degXLabel.setText    (Lang::posX (l), juce::dontSendNotification);
    degYLabel.setText    (Lang::posY (l), juce::dontSendNotification);
    polyXLabel.setText   (Lang::posX (l), juce::dontSendNotification);
    polyYLabel.setText   (Lang::posY (l), juce::dontSendNotification);

    bgColourButton.setButtonText   (Lang::pickColour (l));
    textColourButton.setButtonText (Lang::pickColour (l));
    resetButton.setButtonText      (Lang::resetDefaults (l));

    for (int i = 0; i < modeButtons.size(); ++i)
        modeButtons[i]->setButtonText (Lang::modeName (l, ChordEngine::modes()[(size_t) i].key));
}

void SettingsPanel::changed()
{
    if (updating)
        return;

    auto& s = processor.settings;

    s.midiThru        = midiThruToggle.getToggleState();
    s.captureWindowMs = juce::roundToInt (captureSlider.getValue());

    for (int i = 0; i < accidentalToggles.size(); ++i)
        s.useFlat[ChordEngine::ACCIDENTAL_INDICES[i]] = accidentalToggles[i]->getToggleState();

    s.chordSize   = chordSizeSlider.getValue();
    s.fitToWindow = fitToWindowToggle.getToggleState();

    s.showInversion = showInversionToggle.getToggleState();
    s.invSize = invSizeSlider.getValue();
    s.invX    = invXSlider.getValue();
    s.invY    = invYSlider.getValue();

    s.showDegree = showDegreeToggle.getToggleState();
    s.degSize = degSizeSlider.getValue();
    s.degX    = degXSlider.getValue();
    s.degY    = degYSlider.getValue();

    s.showPoly = showPolyToggle.getToggleState();
    s.polyX    = polyXSlider.getValue();
    s.polyY    = polyYSlider.getValue();

    if (onSettingsChanged)
        onSettingsChanged();
}

void SettingsPanel::refreshFromSettings()
{
    updating = true;
    const auto& s = processor.settings;

    midiThruToggle.setToggleState (s.midiThru, juce::dontSendNotification);
    captureSlider.setValue (s.captureWindowMs, juce::dontSendNotification);

    for (int i = 0; i < accidentalToggles.size(); ++i)
        accidentalToggles[i]->setToggleState (s.useFlat[ChordEngine::ACCIDENTAL_INDICES[i]],
                                              juce::dontSendNotification);

    chordSizeSlider.setValue (s.chordSize, juce::dontSendNotification);
    fitToWindowToggle.setToggleState (s.fitToWindow, juce::dontSendNotification);

    showInversionToggle.setToggleState (s.showInversion, juce::dontSendNotification);
    invSizeSlider.setValue (s.invSize, juce::dontSendNotification);
    invXSlider.setValue (s.invX, juce::dontSendNotification);
    invYSlider.setValue (s.invY, juce::dontSendNotification);

    showDegreeToggle.setToggleState (s.showDegree, juce::dontSendNotification);
    degSizeSlider.setValue (s.degSize, juce::dontSendNotification);
    degXSlider.setValue (s.degX, juce::dontSendNotification);
    degYSlider.setValue (s.degY, juce::dontSendNotification);

    showPolyToggle.setToggleState (s.showPoly, juce::dontSendNotification);
    polyXSlider.setValue (s.polyX, juce::dontSendNotification);
    polyYSlider.setValue (s.polyY, juce::dontSendNotification);

    for (int i = 0; i < languageButtons.size(); ++i)
        styleTextButton (*languageButtons[i], s.language == i);

    for (int i = 0; i < tonicButtons.size(); ++i)
        styleTextButton (*tonicButtons[i], s.tonicPc == i);

    for (int i = 0; i < modeButtons.size(); ++i)
        styleTextButton (*modeButtons[i], s.modeKey == ChordEngine::modes()[(size_t) i].key);

    // Кнопки-образцы показывают текущий цвет
    const juce::Colour bg (s.backgroundColour);
    const juce::Colour tx (s.textColour);
    bgColourButton.setColour (juce::TextButton::buttonColourId, bg);
    bgColourButton.setColour (juce::TextButton::textColourOffId, bg.contrasting (0.7f));
    textColourButton.setColour (juce::TextButton::buttonColourId, tx);
    textColourButton.setColour (juce::TextButton::textColourOffId, tx.contrasting (0.7f));

    styleTextButton (resetButton, false);

    updating = false;
    repaint();
}

void SettingsPanel::paint (juce::Graphics& g)
{
    g.fillAll (colours::panel);

    for (auto& r : sectionBounds)
    {
        g.setColour (colours::section);
        g.fillRoundedRectangle (r.toFloat(), 10.0f);
        g.setColour (colours::sectionEdge);
        g.drawRoundedRectangle (r.toFloat(), 10.0f, 1.0f);
    }

    g.setColour (colours::dimLabel);
    g.setFont (juce::Font (juce::FontOptions (12.0f)));

    for (auto& t : titles)
        g.drawText (t.second, t.first, juce::Justification::centredLeft);
}

void SettingsPanel::resized()
{
    const int l = processor.settings.language;

    sectionBounds.clear();
    titles.clear();

    const int pad = 16;
    const int rowH = 26;
    const int gap = 8;
    int y = pad;
    const int w = juce::jmax (320, getWidth() - pad * 2);
    const int labelW = juce::jmin (250, w / 2);

    auto sectionStart = [&] { return y; };
    auto closeSection = [&] (int startY)
    {
        sectionBounds.add ({ pad - 8, startY - 8, w + 16, y - startY + 16 });
        y += 24;
    };

    auto addTitle = [&] (const juce::String& text)
    {
        titles.add ({ { pad, y, w, 16 }, text });
        y += 22;
    };

    auto addSliderRow = [&] (juce::Label& lb, juce::Slider& s)
    {
        lb.setBounds (pad, y, labelW, rowH);
        s.setBounds (pad + labelW, y, w - labelW, rowH);
        y += rowH + gap;
    };

    // ---- Язык ----
    int st = sectionStart();
    addTitle (Lang::secLanguage (l));
    {
        const int bw = (w - 8) / 3;
        int x = pad;
        for (auto* b : languageButtons) { b->setBounds (x, y, bw - 4, rowH); x += bw; }
        y += rowH + gap;
    }
    closeSection (st);

    // ---- Общее ----
    st = sectionStart();
    addTitle (Lang::secGeneral (l));
    midiThruToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (captureLabel, captureSlider);
    closeSection (st);

    // ---- Строка аккорда ----
    st = sectionStart();
    addTitle (Lang::secChordLine (l));
    fitToWindowToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (chordSizeLabel, chordSizeSlider);
    closeSection (st);

    // ---- Цвета ----
    st = sectionStart();
    addTitle (Lang::secColours (l));
    bgColourLabel.setBounds (pad, y, labelW, rowH);
    bgColourButton.setBounds (pad + labelW, y, w - labelW, rowH);
    y += rowH + gap;
    textColourLabel.setBounds (pad, y, labelW, rowH);
    textColourButton.setBounds (pad + labelW, y, w - labelW, rowH);
    y += rowH + gap;
    closeSection (st);

    // ---- Бемоли ----
    st = sectionStart();
    addTitle (Lang::secFlats (l));
    {
        const int bw = juce::jmax (95, w / 5);
        int x = pad;
        for (auto* b : accidentalToggles)
        {
            if (x + bw > pad + w) { x = pad; y += rowH + 4; }
            b->setBounds (x, y, bw, rowH);
            x += bw;
        }
        y += rowH + gap;
    }
    closeSection (st);

    // ---- Тоника и лад ----
    st = sectionStart();
    addTitle (Lang::secTonic (l));
    {
        const int bw = juce::jmax (46, (w - 11 * 4) / 12);
        int x = pad;
        for (auto* b : tonicButtons)
        {
            if (x + bw > pad + w) { x = pad; y += rowH + 4; }
            b->setBounds (x, y, bw, rowH);
            x += bw + 4;
        }
        y += rowH + gap + 6;
    }
    addTitle (Lang::secMode (l));
    {
        int x = pad;
        for (auto* b : modeButtons)
        {
            const int bw = juce::jmin (w, 140);
            if (x + bw > pad + w) { x = pad; y += rowH + 4; }
            b->setBounds (x, y, bw, rowH);
            x += bw + 4;
        }
        y += rowH + gap;
    }
    closeSection (st);

    // ---- Номер обращения ----
    st = sectionStart();
    addTitle (Lang::secInversion (l));
    showInversionToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (invSizeLabel, invSizeSlider);
    addSliderRow (invXLabel,    invXSlider);
    addSliderRow (invYLabel,    invYSlider);
    closeSection (st);

    // ---- Ступень лада ----
    st = sectionStart();
    addTitle (Lang::secDegree (l));
    showDegreeToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (degSizeLabel, degSizeSlider);
    addSliderRow (degXLabel,    degXSlider);
    addSliderRow (degYLabel,    degYSlider);
    closeSection (st);

    // ---- Полиаккорд ----
    st = sectionStart();
    addTitle (Lang::secPoly (l));
    showPolyToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (polyXLabel, polyXSlider);
    addSliderRow (polyYLabel, polyYSlider);
    closeSection (st);

    // ---- Кнопка сброса, в самом низу ----
    resetButton.setBounds (pad, y, w, rowH + 8);
    y += rowH + 8;

    requiredHeight = y + pad;
}

//==============================================================================
ChordsDetectorEditor::ChordsDetectorEditor (ChordsDetectorProcessor& p)
    : AudioProcessorEditor (&p), processor (p), settingsPanel (p)
{
    typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::PTSansBold_ttf,
                                                        BinaryData::PTSansBold_ttfSize);

    settingsViewport.setViewedComponent (&settingsPanel, false);
    settingsViewport.setScrollBarsShown (true, false);
    settingsViewport.setVisible (false);
    addAndMakeVisible (settingsViewport);

    // Полоса прокрутки в тон панели, а не стандартная синяя
    auto& bar = settingsViewport.getVerticalScrollBar();
    bar.setColour (juce::ScrollBar::thumbColourId,      colours::scrollThumb);
    bar.setColour (juce::ScrollBar::trackColourId,      colours::buttonOff);
    bar.setColour (juce::ScrollBar::backgroundColourId, colours::panel);

    settingsPanel.onSettingsChanged = [this] { refreshEverything(); };

    settingsButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff262626));
    settingsButton.setColour (juce::TextButton::textColourOffId, colours::label);
    settingsButton.onClick = [this] { toggleSettings(); };
    addAndMakeVisible (settingsButton);

    // Размер запоминаем ДО setResizeLimits: он подгоняет окно под свои границы
    // и успевает перезаписать сохранённое значение
    const int savedWidth  = juce::jlimit (400, 4000, processor.lastEditorWidth);
    const int savedHeight = juce::jlimit (200, 3000, processor.lastEditorHeight);

    setResizable (true, true);
    setResizeLimits (400, 200, 4000, 3000);
    setSize (savedWidth, savedHeight);

    // Панель настроек открывается в том же состоянии, в каком её оставили
    settingsViewport.setVisible (processor.settings.showSettings);

    ready = true;
    resized();

    startTimerHz (30);
    refreshEverything();
}

ChordsDetectorEditor::~ChordsDetectorEditor()
{
    if (ready)
    {
        processor.lastEditorWidth  = getWidth();
        processor.lastEditorHeight = getHeight();
    }
}

void ChordsDetectorEditor::refreshEverything()
{
    settingsButton.setButtonText (Lang::settings (processor.settings.language));
    recalculate();
    repaint();
}

void ChordsDetectorEditor::toggleSettings()
{
    const bool show = ! settingsViewport.isVisible();
    settingsViewport.setVisible (show);
    processor.settings.showSettings = show;
    if (show)
    {
        settingsPanel.updateTexts();
        settingsPanel.refreshFromSettings();
    }
    resized();
    repaint();
}

void ChordsDetectorEditor::timerCallback()
{
    const int counter = processor.getUpdateCounter();
    if (counter != lastUpdateCounter)
    {
        lastUpdateCounter = counter;
        recalculate();
        repaint();
    }
}

void ChordsDetectorEditor::recalculate()
{
    const auto& s = processor.settings;

    ChordEngine::NoteNaming naming;
    for (int i = 0; i < 12; ++i)
        naming.useFlat[i] = s.useFlat[i];

    const auto notes = processor.getHeldNotes();
    const auto result = ChordEngine::detect (notes, naming, s.tonicPc, s.modeKey);

    chordTextIsPlaceholder = false;

    if (notes.empty())
    {
        if (processor.hasPlayedAnyNote())
        {
            chordText.clear();
        }
        else
        {
            chordText = Lang::pressANote (s.language);
            chordTextIsPlaceholder = true;
        }

        inversionText.clear();
        degreeText.clear();
        polyText.clear();
        return;
    }

    chordText     = result.text;
    inversionText = s.showInversion ? ChordEngine::inversionText (result) : juce::String();
    degreeText    = s.showDegree    ? ChordEngine::degreeText (result, s.tonicPc, s.modeKey)
                                    : juce::String();
    polyText      = s.showPoly      ? ChordEngine::polychordText (result, naming) : juce::String();
}

void ChordsDetectorEditor::paint (juce::Graphics& g)
{
    const auto& s = processor.settings;

    g.fillAll (juce::Colour (s.backgroundColour));
    g.setColour (juce::Colour (s.textColour));

    const float w = (float) getWidth();
    const float h = (float) getHeight();

    // Подсказка рисуется системным шрифтом: во встроенном PT Sans нет,
    // например, китайских иероглифов
    if (chordTextIsPlaceholder)
    {
        g.setFont (juce::Font (juce::FontOptions (juce::jlimit (12.0f, 40.0f, h * 0.09f))));
        g.drawText (chordText, getLocalBounds(), juce::Justification::centred, false);
        return;
    }

    // ---- Основная строка (аккорд) ----
    if (chordText.isNotEmpty())
    {
        float height = (float) (h * 0.45 * s.chordSize / 100.0);

        if (s.fitToWindow)
        {
            // подбираем размер так, чтобы текст занял почти всю ширину окна
            juce::Font probe (juce::FontOptions (typeface).withHeight (100.0f));
            const float probeWidth =
                juce::jmax (1.0f, juce::GlyphArrangement::getStringWidth (probe, chordText));
            height = 100.0f * (w * 0.92f) / probeWidth;
            height = juce::jmin (height, h * 0.7f);
            height = (float) (height * s.chordSize / 100.0);
        }

        height = juce::jlimit (6.0f, h * 4.0f, height);

        g.setFont (juce::Font (juce::FontOptions (typeface).withHeight (height)));
        g.drawText (chordText, getLocalBounds(), juce::Justification::centred, false);
    }

    // ---- Дополнительные строки ----
    auto drawLine = [&] (const juce::String& text, double sizePercent,
                         double xPercent, double yPercent)
    {
        if (text.isEmpty())
            return;

        const float height = juce::jlimit (6.0f, h * 2.0f,
                                           (float) (h * 0.12 * sizePercent / 100.0));

        g.setFont (juce::Font (juce::FontOptions (typeface).withHeight (height)));

        const int cx = juce::roundToInt (w * xPercent / 100.0);
        const int cy = juce::roundToInt (h * yPercent / 100.0);
        const int bw = getWidth();
        const int bh = juce::roundToInt (height * 1.4f);

        g.drawText (text, cx - bw / 2, cy - bh / 2, bw, bh,
                    juce::Justification::centred, false);
    };

    drawLine (inversionText, s.invSize, s.invX, s.invY);
    drawLine (degreeText,    s.degSize, s.degX, s.degY);
    // Полиаккорд масштабируется вместе с основной строкой — как в веб-версии
    drawLine (polyText,      s.chordSize, s.polyX, s.polyY);
}

void ChordsDetectorEditor::resized()
{
    settingsButton.setBounds (getWidth() - 126, 6, 120, 26);

    if (settingsViewport.isVisible())
    {
        const int panelW = juce::jmin (getWidth() - 20, 580);
        settingsViewport.setBounds ((getWidth() - panelW) / 2, 40,
                                    panelW, juce::jmax (100, getHeight() - 60));
        settingsPanel.setSize (panelW - settingsViewport.getScrollBarThickness(), 100);
        settingsPanel.resized();
        settingsPanel.setSize (settingsPanel.getWidth(), settingsPanel.getRequiredHeight());
    }

    if (ready)
    {
        processor.lastEditorWidth  = getWidth();
        processor.lastEditorHeight = getHeight();
    }
}

void ChordsDetectorEditor::mouseDown (const juce::MouseEvent& e)
{
    // Средняя кнопка мыши открывает/закрывает настройки — как в веб-версии
    if (e.mods.isMiddleButtonDown())
        toggleSettings();
}

void ChordsDetectorEditor::mouseWheelMove (const juce::MouseEvent&,
                                           const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY > 0.02f && ! settingsViewport.isVisible())
        toggleSettings();
    else if (wheel.deltaY < -0.02f && settingsViewport.isVisible())
        toggleSettings();
}
