#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ChordEngine.h"

//==============================================================================
//  Настройки плагина. Сохраняются вместе с проектом DAW.
//  Значения по умолчанию, заданные здесь, восстанавливает кнопка
//  "По умолчанию" в самом низу панели настроек.
//==============================================================================
struct Settings
{
    int  language        = 0;      // 0 - русский, 1 - английский, 2 - китайский
    bool midiThru        = true;   // пропускать MIDI дальше по цепочке
    int  captureWindowMs = 50;     // окно захвата аккорда
    bool showSettings    = false;  // была ли открыта панель настроек

    juce::uint32 backgroundColour = 0xff1a1a1a;
    juce::uint32 textColour       = 0xff8e8e8e;

    bool useFlat[12] = { false, true, false, true, false,
                         false, true, false, true, false, true, false };

    int         tonicPc = -1;      // -1 = не выбрана
    juce::String modeKey;          // пусто = не выбран

    // Основная строка (аккорд)
    double chordSize   = 100.0;    // % от базового размера
    bool   fitToWindow = true;     // подгонять аккорд под ширину окна

    // Строка номера обращения
    bool   showInversion = false;
    double invSize = 100.0;
    double invX    = 25.0;         // % от ширины окна
    double invY    = 78.0;         // % от высоты окна

    // Строка ступени лада
    bool   showDegree = false;
    double degSize = 100.0;
    double degX    = 75.0;
    double degY    = 78.0;

    // Строка полиаккорда
    bool   showPoly = false;
    double polyX    = 50.0;
    double polyY    = 92.0;

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& v);
};

//==============================================================================
class ChordsDetectorProcessor : public juce::AudioProcessor
{
public:
    ChordsDetectorProcessor();
    ~ChordsDetectorProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    Settings settings;

    // Размер окна плагина — сохраняется вместе с проектом
    int lastEditorWidth  = 1800;
    int lastEditorHeight = 1600;

    // Ноты последнего зафиксированного аккорда (для редактора)
    std::vector<int> getHeldNotes() const;

    // Счётчик обновлений — редактор перерисовывается только когда он изменился
    int getUpdateCounter() const noexcept { return updateCounter.load(); }

    bool hasPlayedAnyNote() const noexcept { return playedAnyNote.load(); }

private:
    void commitCapturedNotes();

    mutable juce::CriticalSection notesLock;
    std::vector<int> heldNotes;      // зафиксированный аккорд
    std::vector<int> capturedNotes;  // копится в текущем окне захвата

    bool  captureFinished = true;
    int   samplesUntilCommit = 0;
    double currentSampleRate = 44100.0;

    std::atomic<int>  updateCounter { 0 };
    std::atomic<bool> playedAnyNote { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordsDetectorProcessor)
};
