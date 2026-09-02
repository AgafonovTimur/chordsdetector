#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
namespace colours
{
    static const juce::Colour background   { 0xff1a1a1a };
    static const juce::Colour text         { 0xff8e8e8e };
    static const juce::Colour panel        { 0xff141414 };
    static const juce::Colour section      { 0xff1a1a1a };
    static const juce::Colour sectionEdge  { 0xff2a2a2a };
    static const juce::Colour label        { 0xffdddddd };
    static const juce::Colour dimLabel     { 0xff999999 };
    static const juce::Colour buttonOff    { 0xff1f1f1f };
    static const juce::Colour buttonOn     { 0xff6b6b6b };
}

static const char* TONIC_NAMES[12] =
    { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

//==============================================================================
SettingsPanel::SettingsPanel (NotesToWebProcessor& p) : processor (p)
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

    auto setupLabel = [this] (juce::Label& l, const juce::String& text)
    {
        l.setText (text, juce::dontSendNotification);
        l.setColour (juce::Label::textColourId, colours::dimLabel);
        l.setFont (juce::Font (juce::FontOptions (14.0f)));
        addAndMakeVisible (l);
    };

    setupLabel (captureLabel,   "Окно захвата аккорда, мс");
    setupLabel (chordSizeLabel, "Размер аккорда, %");
    setupLabel (invSizeLabel,   "Размер, %");
    setupLabel (invXLabel,      "Позиция по горизонтали, %");
    setupLabel (invYLabel,      "Позиция по вертикали, %");
    setupLabel (degSizeLabel,   "Размер, %");
    setupLabel (degXLabel,      "Позиция по горизонтали, %");
    setupLabel (degYLabel,      "Позиция по вертикали, %");
    setupLabel (polyXLabel,     "Позиция по горизонтали, %");
    setupLabel (polyYLabel,     "Позиция по вертикали, %");

    // Бемоль вместо диеза — отдельно по каждой ноте
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

    // Тоники
    for (int i = 0; i < 12; ++i)
    {
        auto* b = new juce::TextButton (TONIC_NAMES[i]);
        b->setClickingTogglesState (false);
        b->onClick = [this, i]
        {
            processor.settings.tonicPc = (processor.settings.tonicPc == i) ? -1 : i;
            refreshFromSettings();
            changed();
        };
        addAndMakeVisible (b);
        tonicButtons.add (b);
    }

    // Лады
    for (auto& m : ChordEngine::modes())
    {
        const juce::String key (m.key);
        auto* b = new juce::TextButton (m.label);
        b->setClickingTogglesState (false);
        b->onClick = [this, key]
        {
            processor.settings.modeKey = (processor.settings.modeKey == key) ? juce::String() : key;
            refreshFromSettings();
            changed();
        };
        addAndMakeVisible (b);
        modeButtons.add (b);
    }

    refreshFromSettings();
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

    for (int i = 0; i < tonicButtons.size(); ++i)
    {
        const bool on = (s.tonicPc == i);
        tonicButtons[i]->setColour (juce::TextButton::buttonColourId,
                                    on ? colours::buttonOn : colours::buttonOff);
        tonicButtons[i]->setColour (juce::TextButton::textColourOffId,
                                    on ? juce::Colours::white : colours::label);
    }

    for (int i = 0; i < modeButtons.size(); ++i)
    {
        const bool on = (s.modeKey == ChordEngine::modes()[(size_t) i].key);
        modeButtons[i]->setColour (juce::TextButton::buttonColourId,
                                   on ? colours::buttonOn : colours::buttonOff);
        modeButtons[i]->setColour (juce::TextButton::textColourOffId,
                                   on ? juce::Colours::white : colours::label);
    }

    updating = false;
    repaint();
}

void SettingsPanel::paint (juce::Graphics& g)
{
    g.fillAll (colours::panel);

    g.setColour (colours::section);
    for (auto& r : sectionBounds)
    {
        g.setColour (colours::section);
        g.fillRoundedRectangle (r.toFloat(), 10.0f);
        g.setColour (colours::sectionEdge);
        g.drawRoundedRectangle (r.toFloat(), 10.0f, 1.0f);
    }

    g.setColour (colours::dimLabel);
    g.setFont (juce::Font (juce::FontOptions (12.0f)));

    for (auto& l : titles)
        g.drawText (l.second, l.first, juce::Justification::centredLeft);
}

void SettingsPanel::resized()
{
    sectionBounds.clear();
    titles.clear();

    const int pad = 16;
    const int rowH = 26;
    const int gap = 8;
    int y = pad;
    const int w = juce::jmax (320, getWidth() - pad * 2);
    const int labelW = juce::jmin (220, w / 2);

    auto sectionStart = [&] { return y; };
    auto closeSection = [&] (int startY)
    {
        sectionBounds.add ({ pad - 8, startY - 8, w + 16, y - startY + 16 });
        y += 24;
    };

    auto addTitle = [&] (const juce::String& text)
    {
        titles.add ({ { pad, y, w, 16 }, text.toUpperCase() });
        y += 22;
    };

    auto addSliderRow = [&] (juce::Label& l, juce::Slider& s)
    {
        l.setBounds (pad, y, labelW, rowH);
        s.setBounds (pad + labelW, y, w - labelW, rowH);
        y += rowH + gap;
    };

    // ---- Общее ----
    int st = sectionStart();
    addTitle ("Общее");
    midiThruToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (captureLabel, captureSlider);
    closeSection (st);

    // ---- Строка аккорда ----
    st = sectionStart();
    addTitle ("Строка аккорда");
    fitToWindowToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (chordSizeLabel, chordSizeSlider);
    closeSection (st);

    // ---- Бемоли ----
    st = sectionStart();
    addTitle ("Бемоль вместо диеза");
    {
        const int bw = juce::jmax (90, w / 5);
        int x = pad;
        for (auto* b : accidentalToggles)
        {
            b->setBounds (x, y, bw, rowH);
            x += bw;
            if (x + bw > pad + w) { x = pad; y += rowH + 4; }
        }
        y += rowH + gap;
    }
    closeSection (st);

    // ---- Тоника и лад ----
    st = sectionStart();
    addTitle ("Тоника (можно выбрать только одну)");
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
    addTitle ("Лад (можно выбрать только один)");
    {
        int x = pad;
        for (auto* b : modeButtons)
        {
            const int bw = juce::jmin (w, 130);
            if (x + bw > pad + w) { x = pad; y += rowH + 4; }
            b->setBounds (x, y, bw, rowH);
            x += bw + 4;
        }
        y += rowH + gap;
    }
    closeSection (st);

    // ---- Номер обращения ----
    st = sectionStart();
    addTitle ("Номер обращения");
    showInversionToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (invSizeLabel, invSizeSlider);
    addSliderRow (invXLabel,    invXSlider);
    addSliderRow (invYLabel,    invYSlider);
    closeSection (st);

    // ---- Ступень лада ----
    st = sectionStart();
    addTitle ("Ступень лада");
    showDegreeToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (degSizeLabel, degSizeSlider);
    addSliderRow (degXLabel,    degXSlider);
    addSliderRow (degYLabel,    degYSlider);
    closeSection (st);

    // ---- Полиаккорд ----
    st = sectionStart();
    addTitle ("Полиаккорд");
    showPolyToggle.setBounds (pad, y, w, rowH); y += rowH + gap;
    addSliderRow (polyXLabel, polyXSlider);
    addSliderRow (polyYLabel, polyYSlider);
    closeSection (st);

    requiredHeight = y + pad;
}

//==============================================================================
NotesToWebEditor::NotesToWebEditor (NotesToWebProcessor& p)
    : AudioProcessorEditor (&p), processor (p), settingsPanel (p)
{
    typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::PTSansBold_ttf,
                                                        BinaryData::PTSansBold_ttfSize);

    settingsViewport.setViewedComponent (&settingsPanel, false);
    settingsViewport.setScrollBarsShown (true, false);
    settingsViewport.setVisible (false);
    addAndMakeVisible (settingsViewport);

    settingsPanel.onSettingsChanged = [this]
    {
        recalculate();
        repaint();
    };

    settingsButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff262626));
    settingsButton.setColour (juce::TextButton::textColourOffId, colours::label);
    settingsButton.onClick = [this] { toggleSettings(); };
    addAndMakeVisible (settingsButton);

    setResizable (true, true);
    setResizeLimits (400, 200, 4000, 3000);
    setSize (processor.lastEditorWidth, processor.lastEditorHeight);

    startTimerHz (30);
    recalculate();
}

NotesToWebEditor::~NotesToWebEditor()
{
    processor.lastEditorWidth  = getWidth();
    processor.lastEditorHeight = getHeight();
}

void NotesToWebEditor::toggleSettings()
{
    const bool show = ! settingsViewport.isVisible();
    settingsViewport.setVisible (show);
    if (show)
        settingsPanel.refreshFromSettings();
    resized();
    repaint();
}

void NotesToWebEditor::timerCallback()
{
    const int counter = processor.getUpdateCounter();
    if (counter != lastUpdateCounter)
    {
        lastUpdateCounter = counter;
        recalculate();
        repaint();
    }
}

void NotesToWebEditor::recalculate()
{
    const auto& s = processor.settings;

    ChordEngine::NoteNaming naming;
    for (int i = 0; i < 12; ++i)
        naming.useFlat[i] = s.useFlat[i];

    const auto notes = processor.getHeldNotes();
    const auto result = ChordEngine::detect (notes, naming, s.tonicPc, s.modeKey);

    if (notes.empty())
    {
        chordText = processor.hasPlayedAnyNote()
                        ? juce::String()
                        : juce::String (juce::CharPointer_UTF8 ("\xd0\xbd\xd0\xb0\xd0\xb6\xd0\xbc\xd0\xb8\xd1\x82\xd0\xb5 \xd0\xbd\xd0\xbe\xd1\x82\xd1\x83"));
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

void NotesToWebEditor::paint (juce::Graphics& g)
{
    g.fillAll (colours::background);

    if (chordText.isEmpty() && inversionText.isEmpty()
        && degreeText.isEmpty() && polyText.isEmpty())
        return;

    const auto& s = processor.settings;
    const float w = (float) getWidth();
    const float h = (float) getHeight();

    g.setColour (colours::text);

    // ---- Основная строка (аккорд) ----
    if (chordText.isNotEmpty())
    {
        float height = (float) (h * 0.45 * s.chordSize / 100.0);

        if (s.fitToWindow)
        {
            // подбираем размер так, чтобы текст занял почти всю ширину окна
            juce::Font probe (juce::FontOptions (typeface).withHeight (100.0f));
            const float probeWidth = juce::jmax (1.0f, juce::GlyphArrangement::getStringWidth (probe, chordText));
            height = 100.0f * (w * 0.92f) / probeWidth;
            height = juce::jmin (height, h * 0.7f);
            height = (float) (height * s.chordSize / 100.0);
        }

        height = juce::jlimit (6.0f, h * 4.0f, height);

        g.setFont (juce::Font (juce::FontOptions (typeface).withHeight (height)));
        g.drawText (chordText, getLocalBounds(), juce::Justification::centred, false);
    }

    // ---- Дополнительные строки ----
    auto drawLine = [&] (const juce::String& text, double sizePercent, double xPercent, double yPercent)
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

    drawLine (inversionText, s.invSize,  s.invX,  s.invY);
    drawLine (degreeText,    s.degSize,  s.degX,  s.degY);
    // Полиаккорд масштабируется вместе с основной строкой — как в веб-версии
    drawLine (polyText,      s.chordSize, s.polyX, s.polyY);
}

void NotesToWebEditor::resized()
{
    settingsButton.setBounds (getWidth() - 106, 6, 100, 26);

    if (settingsViewport.isVisible())
    {
        const int panelW = juce::jmin (getWidth() - 20, 560);
        settingsViewport.setBounds ((getWidth() - panelW) / 2, 40,
                                    panelW, juce::jmax (100, getHeight() - 60));
        settingsPanel.setSize (panelW - settingsViewport.getScrollBarThickness(), 100);
        settingsPanel.resized();
        settingsPanel.setSize (settingsPanel.getWidth(), settingsPanel.getRequiredHeight());
    }

    processor.lastEditorWidth  = getWidth();
    processor.lastEditorHeight = getHeight();
}

void NotesToWebEditor::mouseDown (const juce::MouseEvent& e)
{
    // Средняя кнопка мыши открывает/закрывает настройки — как в веб-версии
    if (e.mods.isMiddleButtonDown())
        toggleSettings();
}

void NotesToWebEditor::mouseWheelMove (const juce::MouseEvent&,
                                       const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY > 0.02f && ! settingsViewport.isVisible())
        toggleSettings();
    else if (wheel.deltaY < -0.02f && settingsViewport.isVisible())
        toggleSettings();
}
