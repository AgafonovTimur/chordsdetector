#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
namespace ids
{
    #define ID(name) static const juce::Identifier name (#name);
    ID (NotesToWebState)
    ID (midiThru) ID (captureWindowMs)
    ID (useFlat) ID (tonicPc) ID (modeKey)
    ID (chordSize) ID (fitToWindow)
    ID (showInversion) ID (invSize) ID (invX) ID (invY)
    ID (showDegree) ID (degSize) ID (degX) ID (degY)
    ID (showPoly) ID (polyX) ID (polyY)
    #undef ID
}

juce::ValueTree Settings::toValueTree() const
{
    juce::ValueTree v (ids::NotesToWebState);

    v.setProperty (ids::midiThru,        midiThru,        nullptr);
    v.setProperty (ids::captureWindowMs, captureWindowMs, nullptr);

    juce::String flags;
    for (int i = 0; i < 12; ++i)
        flags += useFlat[i] ? "1" : "0";
    v.setProperty (ids::useFlat, flags, nullptr);

    v.setProperty (ids::tonicPc, tonicPc, nullptr);
    v.setProperty (ids::modeKey, modeKey, nullptr);

    v.setProperty (ids::chordSize,   chordSize,   nullptr);
    v.setProperty (ids::fitToWindow, fitToWindow, nullptr);

    v.setProperty (ids::showInversion, showInversion, nullptr);
    v.setProperty (ids::invSize, invSize, nullptr);
    v.setProperty (ids::invX,    invX,    nullptr);
    v.setProperty (ids::invY,    invY,    nullptr);

    v.setProperty (ids::showDegree, showDegree, nullptr);
    v.setProperty (ids::degSize, degSize, nullptr);
    v.setProperty (ids::degX,    degX,    nullptr);
    v.setProperty (ids::degY,    degY,    nullptr);

    v.setProperty (ids::showPoly, showPoly, nullptr);
    v.setProperty (ids::polyX,    polyX,    nullptr);
    v.setProperty (ids::polyY,    polyY,    nullptr);

    return v;
}

void Settings::fromValueTree (const juce::ValueTree& v)
{
    if (! v.isValid())
        return;

    midiThru        = v.getProperty (ids::midiThru,        midiThru);
    captureWindowMs = v.getProperty (ids::captureWindowMs, captureWindowMs);

    const juce::String flags = v.getProperty (ids::useFlat, juce::String());
    if (flags.length() == 12)
        for (int i = 0; i < 12; ++i)
            useFlat[i] = (flags[i] == '1');

    tonicPc = v.getProperty (ids::tonicPc, tonicPc);
    modeKey = v.getProperty (ids::modeKey, modeKey).toString();

    chordSize   = v.getProperty (ids::chordSize,   chordSize);
    fitToWindow = v.getProperty (ids::fitToWindow, fitToWindow);

    showInversion = v.getProperty (ids::showInversion, showInversion);
    invSize = v.getProperty (ids::invSize, invSize);
    invX    = v.getProperty (ids::invX,    invX);
    invY    = v.getProperty (ids::invY,    invY);

    showDegree = v.getProperty (ids::showDegree, showDegree);
    degSize = v.getProperty (ids::degSize, degSize);
    degX    = v.getProperty (ids::degX,    degX);
    degY    = v.getProperty (ids::degY,    degY);

    showPoly = v.getProperty (ids::showPoly, showPoly);
    polyX    = v.getProperty (ids::polyX,    polyX);
    polyY    = v.getProperty (ids::polyY,    polyY);
}

//==============================================================================
NotesToWebProcessor::NotesToWebProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void NotesToWebProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    samplesUntilCommit = 0;
    captureFinished = true;
}

bool NotesToWebProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo()
        || out == juce::AudioChannelSet::mono();
}

void NotesToWebProcessor::commitCapturedNotes()
{
    const juce::ScopedLock sl (notesLock);
    heldNotes = capturedNotes;
    std::sort (heldNotes.begin(), heldNotes.end());
    captureFinished = true;
    samplesUntilCommit = 0;
    updateCounter.fetch_add (1);
}

void NotesToWebProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    // Плагин ничего не озвучивает — выдаём тишину
    buffer.clear();

    const int windowSamples =
        juce::roundToInt (currentSampleRate * (double) settings.captureWindowMs / 1000.0);

    for (const auto meta : midiMessages)
    {
        const auto msg = meta.getMessage();

        // Отпускания клавиш не учитываются: ноты остаются на экране,
        // пока не будет нажата следующая — как в веб-версии
        if (! msg.isNoteOn())
            continue;

        playedAnyNote.store (true);

        {
            const juce::ScopedLock sl (notesLock);

            if (captureFinished)
            {
                capturedNotes.clear();
                captureFinished = false;
            }

            const int note = msg.getNoteNumber();
            if (std::find (capturedNotes.begin(), capturedNotes.end(), note) == capturedNotes.end())
                capturedNotes.push_back (note);
        }

        // Любое новое нажатие перезапускает окно захвата
        samplesUntilCommit = juce::jmax (1, windowSamples);
    }

    if (samplesUntilCommit > 0)
    {
        samplesUntilCommit -= buffer.getNumSamples();
        if (samplesUntilCommit <= 0)
            commitCapturedNotes();
    }

    if (! settings.midiThru)
        midiMessages.clear();
}

std::vector<int> NotesToWebProcessor::getHeldNotes() const
{
    const juce::ScopedLock sl (notesLock);
    return heldNotes;
}

//==============================================================================
juce::AudioProcessorEditor* NotesToWebProcessor::createEditor()
{
    return new NotesToWebEditor (*this);
}

void NotesToWebProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = settings.toValueTree();

    if (auto* ed = dynamic_cast<NotesToWebEditor*> (getActiveEditor()))
    {
        state.setProperty ("editorWidth",  ed->getWidth(),  nullptr);
        state.setProperty ("editorHeight", ed->getHeight(), nullptr);
    }
    else
    {
        state.setProperty ("editorWidth",  lastEditorWidth,  nullptr);
        state.setProperty ("editorHeight", lastEditorHeight, nullptr);
    }

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void NotesToWebProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml (*xml);
        settings.fromValueTree (state);

        lastEditorWidth  = state.getProperty ("editorWidth",  lastEditorWidth);
        lastEditorHeight = state.getProperty ("editorHeight", lastEditorHeight);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NotesToWebProcessor();
}
