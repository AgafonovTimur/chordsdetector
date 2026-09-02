#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

//==============================================================================
//  Панель настроек. Прокручивается, если окно плагина маленькое.
//==============================================================================
class SettingsPanel : public juce::Component
{
public:
    explicit SettingsPanel (NotesToWebProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onSettingsChanged;

    void refreshFromSettings();
    int  getRequiredHeight() const { return requiredHeight; }

private:
    void changed();

    NotesToWebProcessor& processor;

    juce::ToggleButton midiThruToggle { "Пропускать MIDI дальше (MIDI Thru)" };

    juce::Label  captureLabel;
    juce::Slider captureSlider;

    juce::Label  chordSizeLabel;
    juce::Slider chordSizeSlider;
    juce::ToggleButton fitToWindowToggle { "Подгонять аккорд под ширину окна" };

    juce::OwnedArray<juce::ToggleButton> accidentalToggles;
    juce::OwnedArray<juce::TextButton>   tonicButtons;
    juce::OwnedArray<juce::TextButton>   modeButtons;

    juce::ToggleButton showInversionToggle { "Показывать номер обращения" };
    juce::Slider invSizeSlider, invXSlider, invYSlider;
    juce::Label  invSizeLabel, invXLabel, invYLabel;

    juce::ToggleButton showDegreeToggle { "Показывать ступень лада" };
    juce::Slider degSizeSlider, degXSlider, degYSlider;
    juce::Label  degSizeLabel, degXLabel, degYLabel;

    juce::ToggleButton showPolyToggle { "Показывать полиаккорд, если аккорд не определился" };
    juce::Slider polyXSlider, polyYSlider;
    juce::Label  polyXLabel, polyYLabel;

    // Заголовки секций рисуются напрямую, без отдельных компонентов
    juce::Array<std::pair<juce::Rectangle<int>, juce::String>> titles;
    juce::Array<juce::Rectangle<int>> sectionBounds;

    int requiredHeight = 900;
    bool updating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPanel)
};

//==============================================================================
class NotesToWebEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit NotesToWebEditor (NotesToWebProcessor&);
    ~NotesToWebEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override;
    void recalculate();
    void toggleSettings();

    NotesToWebProcessor& processor;

    juce::Typeface::Ptr typeface;

    juce::Viewport      settingsViewport;
    SettingsPanel       settingsPanel;
    juce::TextButton    settingsButton { "Настройки" };

    // Текущее содержимое строк
    juce::String chordText, inversionText, degreeText, polyText;

    int lastUpdateCounter = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotesToWebEditor)
};
