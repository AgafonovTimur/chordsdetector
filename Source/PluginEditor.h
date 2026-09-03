#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "Lang.h"

//==============================================================================
//  Панель настроек. Прокручивается, если окно плагина маленькое.
//==============================================================================
class SettingsPanel : public juce::Component,
                      private juce::ChangeListener
{
public:
    explicit SettingsPanel (ChordsDetectorProcessor& p);
    ~SettingsPanel() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onSettingsChanged;

    void refreshFromSettings();
    void updateTexts();
    int  getRequiredHeight() const { return requiredHeight; }

private:
    void changed();
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void openColourPicker (int target, juce::Component& anchor);
    void styleTextButton (juce::TextButton& b, bool on);

    ChordsDetectorProcessor& processor;

    juce::OwnedArray<juce::TextButton> languageButtons;

    juce::Label  captureLabel;
    juce::Slider captureSlider;

    juce::Label  chordSizeLabel;
    juce::Slider chordSizeSlider;
    juce::ToggleButton fitToWindowToggle;

    juce::Label      bgColourLabel, textColourLabel;
    juce::TextButton bgColourButton, textColourButton;
    int activeColourTarget = -1;   // 0 — фон, 1 — текст

    juce::OwnedArray<juce::ToggleButton> accidentalToggles;
    juce::OwnedArray<juce::TextButton>   tonicButtons;
    juce::OwnedArray<juce::TextButton>   modeButtons;

    juce::ToggleButton showInversionToggle;
    juce::Slider invSizeSlider, invXSlider, invYSlider;
    juce::Label  invSizeLabel, invXLabel, invYLabel;

    juce::ToggleButton showDegreeToggle;
    juce::Slider degSizeSlider, degXSlider, degYSlider;
    juce::Label  degSizeLabel, degXLabel, degYLabel;

    juce::ToggleButton showPolyToggle;
    juce::Slider polyXSlider, polyYSlider;
    juce::Label  polyXLabel, polyYLabel;

    juce::TextButton resetButton;

    // Заголовки секций рисуются напрямую, без отдельных компонентов
    juce::Array<std::pair<juce::Rectangle<int>, juce::String>> titles;
    juce::Array<juce::Rectangle<int>> sectionBounds;

    int requiredHeight = 900;
    bool updating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPanel)
};

//==============================================================================
class ChordsDetectorEditor : public juce::AudioProcessorEditor,
                             private juce::Timer
{
public:
    explicit ChordsDetectorEditor (ChordsDetectorProcessor&);
    ~ChordsDetectorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    void refreshEverything();

private:
    void timerCallback() override;
    void recalculate();
    void toggleSettings();

    ChordsDetectorProcessor& processor;

    juce::Typeface::Ptr typeface;

    juce::Viewport      settingsViewport;
    SettingsPanel       settingsPanel;
    juce::TextButton    settingsButton;

    // Текущее содержимое строк
    juce::String chordText, inversionText, degreeText, polyText;
    bool chordTextIsPlaceholder = false;

    int lastUpdateCounter = -1;

    // Пока окно не построено до конца, его размер в настройки не записывается:
    // иначе ограничения размера успевают затереть сохранённое значение
    bool ready = false;

    // Кнопка настроек прячется через 5 секунд после открытия окна
    // и дальше показывается только при наведении мыши
    static constexpr int buttonHideDelayMs = 5000;
    juce::uint32 editorOpenedAtMs = 0;
    bool buttonAutoHidden = false;
    bool buttonShown = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordsDetectorEditor)
};
