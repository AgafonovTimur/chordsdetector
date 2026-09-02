#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <set>
#include <algorithm>

//==============================================================================
//  ChordEngine — распознавание аккордов.
//  Таблицы и логика перенесены один в один из веб-версии Notes_To_Web.
//  Совпадение СТРОГОЕ: набор нажатых нот должен совпасть с шаблоном нота в ноту.
//==============================================================================
namespace ChordEngine
{
    static const char* NOTE_NAMES_SHARP[12] =
        { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    static const char* NOTE_NAMES_FLAT[12] =
        { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

    // Ноты, для которых можно независимо выбрать диез или бемоль
    static const int ACCIDENTAL_INDICES[5] = { 1, 3, 6, 8, 10 };

    struct Template
    {
        juce::String name;          // суффикс: C + "maj7" = "Cmaj7"
        std::vector<int> intervals; // интервалы от корня, по возрастанию
    };

    struct Quality
    {
        bool upperCase;             // true — заглавная римская цифра (мажорная основа)
        juce::String symbol;        // "", "+" или "°"
    };

    // ====== ШАБЛОНЫ АККОРДОВ (правьте здесь, если надо добавить аккорд) ======
    inline const std::vector<Template>& templates()
    {
        static const std::vector<Template> t = {
    { "13", { 0, 2, 4, 5, 7, 9, 10 } },
    { "maj13", { 0, 2, 4, 5, 7, 9, 11 } },
    { "m13", { 0, 2, 3, 5, 7, 9, 10 } },
    { "13", { 0, 2, 4, 7, 9, 10 } },
    { "maj13", { 0, 2, 4, 7, 9, 11 } },
    { "m13", { 0, 2, 3, 7, 9, 10 } },
    { "11", { 0, 2, 4, 5, 7, 10 } },
    { "maj11", { 0, 2, 4, 5, 7, 11 } },
    { "m11", { 0, 2, 3, 5, 7, 10 } },
    { "m(maj11)", { 0, 2, 3, 5, 7, 11 } },
    { "7alt", { 0, 1, 3, 4, 6, 10 } },
    { "7alt", { 0, 1, 3, 4, 8, 10 } },
    { "13(#11)", { 0, 2, 4, 6, 9, 10 } },
    { "maj13(#11)", { 0, 2, 4, 6, 9, 11 } },
    { "13", { 0, 2, 4, 9, 10 } },
    { "maj13", { 0, 2, 4, 9, 11 } },
    { "m13", { 0, 2, 3, 9, 10 } },
    { "9", { 0, 2, 4, 7, 10 } },
    { "maj9", { 0, 2, 4, 7, 11 } },
    { "m9", { 0, 2, 3, 7, 10 } },
    { "m(maj9)", { 0, 2, 3, 7, 11 } },
    { "9sus4", { 0, 2, 5, 7, 10 } },
    { "6/9", { 0, 2, 4, 7, 9 } },
    { "m6/9", { 0, 2, 3, 7, 9 } },
    { "7(add11)", { 0, 4, 5, 7, 10 } },
    { "m7(add11)", { 0, 3, 5, 7, 10 } },
    { "7(b9)", { 0, 1, 4, 7, 10 } },
    { "7(#9)", { 0, 3, 4, 7, 10 } },
    { "7(#11)", { 0, 4, 6, 7, 10 } },
    { "9(#11)", { 0, 2, 4, 6, 10 } },
    { "maj9(#11)", { 0, 2, 4, 6, 11 } },
    { "9(#5)", { 0, 2, 4, 8, 10 } },
    { "7(b13)", { 0, 4, 7, 8, 10 } },
    { "maj7(#11)", { 0, 4, 6, 7, 11 } },
    { "7(b5b9)", { 0, 1, 4, 6, 10 } },
    { "7(b5#9)", { 0, 3, 4, 6, 10 } },
    { "7(#5b9)", { 0, 1, 4, 8, 10 } },
    { "7(#5#9)", { 0, 3, 4, 8, 10 } },
    { "7", { 0, 4, 7, 10 } },
    { "maj7", { 0, 4, 7, 11 } },
    { "m7", { 0, 3, 7, 10 } },
    { "m7(b5)", { 0, 3, 6, 10 } },
    { "dim7", { 0, 3, 6, 9 } },
    { "m(maj7)", { 0, 3, 7, 11 } },
    { "dim(maj7)", { 0, 3, 6, 11 } },
    { "maj7(#5)", { 0, 4, 8, 11 } },
    { "7(#5)", { 0, 4, 8, 10 } },
    { "7(b5)", { 0, 4, 6, 10 } },
    { "7sus4", { 0, 5, 7, 10 } },
    { "7sus2", { 0, 2, 7, 10 } },
    { "6", { 0, 4, 7, 9 } },
    { "m6", { 0, 3, 7, 9 } },
    { "add9", { 0, 2, 4, 7 } },
    { "m(add9)", { 0, 2, 3, 7 } },
    { "add11", { 0, 4, 5, 7 } },
    { "m(add11)", { 0, 3, 5, 7 } },
    { "9(no5)", { 0, 2, 4, 10 } },
    { "maj9(no5)", { 0, 2, 4, 11 } },
    { "m9(no5)", { 0, 2, 3, 10 } },
    { "13(no5)", { 0, 4, 9, 10 } },
    { "m13(no5)", { 0, 3, 9, 10 } },
    { "7(b9)(no5)", { 0, 1, 4, 10 } },
    { "7(#9)(no5)", { 0, 3, 4, 10 } },
    { "", { 0, 4, 7 } },
    { "m", { 0, 3, 7 } },
    { "dim", { 0, 3, 6 } },
    { "aug", { 0, 4, 8 } },
    { "sus2", { 0, 2, 7 } },
    { "sus4", { 0, 5, 7 } },
    { "7(no5)", { 0, 4, 10 } },
    { "m7(no5)", { 0, 3, 10 } },
    { "maj7(no5)", { 0, 4, 11 } },
    { "m(maj7)(no5)", { 0, 3, 11 } },
    { "5", { 0, 7 } },
        };
        return t;
    }

    // ====== КАЧЕСТВО АККОРДА для римской цифры ступени лада ======
    inline Quality qualityFor (const juce::String& name)
    {
        struct Entry { const char* name; Quality q; };
        static const std::vector<Entry> map = {
    { "13", { true, "" } },
    { "maj13", { true, "" } },
    { "m13", { false, "" } },
    { "11", { true, "" } },
    { "maj11", { true, "" } },
    { "m11", { false, "" } },
    { "m(maj11)", { false, "" } },
    { "7alt", { true, "" } },
    { "9", { true, "" } },
    { "maj9", { true, "" } },
    { "m9", { false, "" } },
    { "m(maj9)", { false, "" } },
    { "9sus4", { true, "" } },
    { "6/9", { true, "" } },
    { "m6/9", { false, "" } },
    { "7(add11)", { true, "" } },
    { "m7(add11)", { false, "" } },
    { "7(b9)", { true, "" } },
    { "7(#9)", { true, "" } },
    { "7(#11)", { true, "" } },
    { "9(#11)", { true, "" } },
    { "maj9(#11)", { true, "" } },
    { "9(#5)", { true, "+" } },
    { "13(#11)", { true, "" } },
    { "maj13(#11)", { true, "" } },
    { "7(b13)", { true, "" } },
    { "maj7(#11)", { true, "" } },
    { "7(b5b9)", { true, "" } },
    { "7(b5#9)", { true, "" } },
    { "7(#5b9)", { true, "+" } },
    { "7(#5#9)", { true, "+" } },
    { "7", { true, "" } },
    { "maj7", { true, "" } },
    { "m7", { false, "" } },
    { "m7(b5)", { false, "°" } },
    { "dim7", { false, "°" } },
    { "m(maj7)", { false, "" } },
    { "dim(maj7)", { false, "°" } },
    { "maj7(#5)", { true, "+" } },
    { "7(#5)", { true, "+" } },
    { "7(b5)", { true, "" } },
    { "7sus4", { true, "" } },
    { "7sus2", { true, "" } },
    { "6", { true, "" } },
    { "m6", { false, "" } },
    { "add9", { true, "" } },
    { "m(add9)", { false, "" } },
    { "add11", { true, "" } },
    { "m(add11)", { false, "" } },
    { "9(no5)", { true, "" } },
    { "maj9(no5)", { true, "" } },
    { "m9(no5)", { false, "" } },
    { "13(no5)", { true, "" } },
    { "m13(no5)", { false, "" } },
    { "7(b9)(no5)", { true, "" } },
    { "7(#9)(no5)", { true, "" } },
    { "", { true, "" } },
    { "m", { false, "" } },
    { "dim", { false, "°" } },
    { "aug", { true, "+" } },
    { "sus2", { true, "" } },
    { "sus4", { true, "" } },
    { "7(no5)", { true, "" } },
    { "m7(no5)", { false, "" } },
    { "maj7(no5)", { true, "" } },
    { "m(maj7)(no5)", { false, "" } },
    { "5", { true, "" } },
        };
        for (auto& e : map)
            if (name == e.name)
                return e.q;
        return { true, "" };
    }

    // ====== ЛАДЫ ======
    struct Mode { const char* key; const char* label; std::vector<int> intervals; };

    inline const std::vector<Mode>& modes()
    {
        static const std::vector<Mode> m = {
            { "major",    "Мажор",          { 0, 2, 4, 5, 7, 9, 11 } },
            { "natmin",   "Натур. минор",   { 0, 2, 3, 5, 7, 8, 10 } },
            { "harmin",   "Гарм. минор",    { 0, 2, 3, 5, 7, 8, 11 } },
            { "melmin",   "Мелод. минор",   { 0, 2, 3, 5, 7, 9, 11 } },
            { "dorian",   "Дорийский",      { 0, 2, 3, 5, 7, 9, 10 } },
            { "phrygian", "Фригийский",     { 0, 1, 3, 5, 7, 8, 10 } },
            { "lydian",   "Лидийский",      { 0, 2, 4, 6, 7, 9, 11 } },
            { "mixo",     "Миксолидийский", { 0, 2, 4, 5, 7, 9, 10 } },
            { "locrian",  "Локрийский",     { 0, 1, 3, 5, 6, 8, 10 } },
        };
        return m;
    }

    static const char* ROMAN_NUMERALS[7] = { "I", "II", "III", "IV", "V", "VI", "VII" };

    // ====== ПОЛИАККОРДЫ (Upper Structure Triads) ======
    // Нижняя часть строится от басовой ноты, порядок = приоритет (от полных к простым)
    inline const std::vector<Template>& polyLower()
    {
        static const std::vector<Template> t = {
            { "7",            { 0, 4, 7, 10 } },
            { "m7",           { 0, 3, 7, 10 } },
            { "maj7",         { 0, 4, 7, 11 } },
            { "m(maj7)",      { 0, 3, 7, 11 } },
            { "7(no5)",       { 0, 4, 10 } },
            { "m7(no5)",      { 0, 3, 10 } },
            { "maj7(no5)",    { 0, 4, 11 } },
            { "m(maj7)(no5)", { 0, 3, 11 } },
            { "",             { 0, 4, 7 } },
            { "m",            { 0, 3, 7 } },
            { "5",            { 0, 7 } },
            { "",             { 0 } },
        };
        return t;
    }

    // Верхнее трезвучие, порядок = приоритет
    inline const std::vector<Template>& polyUpper()
    {
        static const std::vector<Template> t = {
            { "",    { 0, 4, 7 } },
            { "m",   { 0, 3, 7 } },
            { "aug", { 0, 4, 8 } },
            { "dim", { 0, 3, 6 } },
        };
        return t;
    }

    //==========================================================================
    struct NoteNaming
    {
        bool useFlat[12] = { false, true, false, true, false,
                             false, true, false, true, false, true, false };

        juce::String name (int pitchClass) const
        {
            pitchClass = ((pitchClass % 12) + 12) % 12;
            return useFlat[pitchClass] ? NOTE_NAMES_FLAT[pitchClass]
                                       : NOTE_NAMES_SHARP[pitchClass];
        }
    };

    struct Result
    {
        juce::String text;              // основная строка
        bool matched = false;           // аккорд распознан
        bool isSingle = false;          // одна нота
        int root = -1;                  // корень аккорда (0..11)
        juce::String templateName;      // суффикс шаблона
        int bassPc = -1;                // басовая нота (0..11)
        std::vector<int> orderedPcs;    // ноты от корня — для номера обращения
        std::vector<int> pitchClasses;  // все ноты набора
    };

    //==========================================================================
    // Ступень лада: индекс 0..6 или -1, если нота не входит в лад
    inline int degreeIndexInScale (int rootPc, int tonicPc, const juce::String& modeKey)
    {
        if (tonicPc < 0 || modeKey.isEmpty())
            return -1;

        for (auto& m : modes())
        {
            if (modeKey == m.key)
            {
                for (size_t i = 0; i < m.intervals.size(); ++i)
                    if ((tonicPc + m.intervals[i]) % 12 == rootPc)
                        return (int) i;
                return -1;
            }
        }
        return -1;
    }

    //==========================================================================
    // Выбор лучшего варианта, когда один набор нот читается несколькими способами
    // (например C Eb G Bb — это и Cm7, и Eb6):
    //   1) предпочитаем вариант, чей корень входит в выбранный лад
    //   2) затем — вариант, чей корень совпадает с басовой нотой
    //   3) иначе первый найденный
    struct Candidate { const Template* tmpl; int root; };

    inline Candidate pickBest (const std::vector<Candidate>& candidates,
                               int bassPc, int tonicPc, const juce::String& modeKey)
    {
        if (candidates.size() == 1)
            return candidates[0];

        std::vector<Candidate> pool = candidates;

        if (tonicPc >= 0 && modeKey.isNotEmpty())
        {
            std::vector<Candidate> diatonic;
            for (auto& c : pool)
                if (degreeIndexInScale (c.root, tonicPc, modeKey) >= 0)
                    diatonic.push_back (c);

            if (diatonic.size() == 1) return diatonic[0];
            if (diatonic.size() > 1)  pool = diatonic;
        }

        std::vector<Candidate> byBass;
        for (auto& c : pool)
            if (c.root == bassPc)
                byBass.push_back (c);

        if (byBass.size() == 1) return byBass[0];
        if (byBass.size() > 1)  pool = byBass;

        return pool[0];
    }

    //==========================================================================
    inline Result detect (const std::vector<int>& midiNotes,
                          const NoteNaming& naming,
                          int tonicPc,
                          const juce::String& modeKey)
    {
        Result r;

        if (midiNotes.empty())
            return r;

        const int bassNote = *std::min_element (midiNotes.begin(), midiNotes.end());
        r.bassPc = bassNote % 12;

        // Уникальные ноты (без учёта октавы), в порядке появления — как в веб-версии
        std::vector<int> pcs;
        for (int n : midiNotes)
        {
            const int pc = n % 12;
            if (std::find (pcs.begin(), pcs.end(), pc) == pcs.end())
                pcs.push_back (pc);
        }
        r.pitchClasses = pcs;

        if (pcs.size() == 1)
        {
            r.text = naming.name (pcs[0]);
            r.matched = true;
            r.isSingle = true;
            r.root = pcs[0];
            r.orderedPcs = pcs;
            return r;
        }

        // Шаблоны от самых больших к самым простым
        std::vector<const Template*> sorted;
        for (auto& t : templates())
            sorted.push_back (&t);
        std::stable_sort (sorted.begin(), sorted.end(),
                          [] (const Template* a, const Template* b)
                          { return a->intervals.size() > b->intervals.size(); });

        std::vector<Candidate> candidates;
        for (auto* t : sorted)
        {
            if (t->intervals.size() != pcs.size())
                continue;

            for (int root : pcs)
            {
                std::vector<int> rel;
                for (int pc : pcs)
                    rel.push_back (((pc - root) % 12 + 12) % 12);
                std::sort (rel.begin(), rel.end());

                if (rel == t->intervals)
                    candidates.push_back ({ t, root });
            }
        }

        if (! candidates.empty())
        {
            const auto chosen = pickBest (candidates, r.bassPc, tonicPc, modeKey);

            r.matched = true;
            r.isSingle = false;
            r.root = chosen.root;
            r.templateName = chosen.tmpl->name;
            r.text = naming.name (chosen.root) + chosen.tmpl->name;

            r.orderedPcs = pcs;
            const int root = chosen.root;
            std::sort (r.orderedPcs.begin(), r.orderedPcs.end(),
                       [root] (int a, int b)
                       { return ((a - root) % 12 + 12) % 12 < ((b - root) % 12 + 12) % 12; });
            return r;
        }

        // Не распознано — перечисляем ноты
        juce::StringArray parts;
        for (int pc : pcs)
            parts.add (naming.name (pc));
        r.text = parts.joinIntoString (" ");
        r.matched = false;
        return r;
    }

    //==========================================================================
    // Номер обращения: "" для основного вида, иначе 2, 3, 4...
    inline juce::String inversionText (const Result& r)
    {
        if (! r.matched || r.isSingle)
            return {};

        const auto it = std::find (r.orderedPcs.begin(), r.orderedPcs.end(), r.bassPc);
        if (it == r.orderedPcs.end())
            return {};

        const int idx = (int) std::distance (r.orderedPcs.begin(), it);
        if (idx <= 0)
            return {};

        return juce::String (idx + 1);
    }

    //==========================================================================
    // Ступень лада римской цифрой
    inline juce::String degreeText (const Result& r, int tonicPc, const juce::String& modeKey)
    {
        if (! r.matched)
            return {};

        const int idx = degreeIndexInScale (r.root, tonicPc, modeKey);
        if (idx < 0)
            return {};

        const auto q = r.isSingle ? Quality { true, "" } : qualityFor (r.templateName);
        juce::String numeral (ROMAN_NUMERALS[idx]);
        if (! q.upperCase)
            numeral = numeral.toLowerCase();

        return numeral + q.symbol;
    }

    //==========================================================================
    // Полиаккорд: трезвучие поверх нижней части от басовой ноты.
    // Используется только тогда, когда обычный аккорд не определился.
    inline juce::String polychordText (const Result& r, const NoteNaming& naming)
    {
        if (r.matched || r.pitchClasses.size() < 3)
            return {};

        const std::set<int> set (r.pitchClasses.begin(), r.pitchClasses.end());

        for (auto& low : polyLower())
        {
            std::vector<int> lowPcs;
            for (int i : low.intervals)
                lowPcs.push_back ((r.bassPc + i) % 12);

            bool lowOk = true;
            for (int pc : lowPcs)
                if (set.find (pc) == set.end()) { lowOk = false; break; }
            if (! lowOk)
                continue;

            for (auto& up : polyUpper())
            {
                for (int upRoot = 0; upRoot < 12; ++upRoot)
                {
                    if (upRoot == r.bassPc)
                        continue; // трезвучие от самого баса — это не полиаккорд

                    std::vector<int> upPcs;
                    for (int i : up.intervals)
                        upPcs.push_back ((upRoot + i) % 12);

                    bool upOk = true;
                    for (int pc : upPcs)
                        if (set.find (pc) == set.end()) { upOk = false; break; }
                    if (! upOk)
                        continue;

                    // верхнее трезвучие должно добавлять хотя бы одну свою ноту
                    bool addsSomething = false;
                    for (int pc : upPcs)
                        if (std::find (lowPcs.begin(), lowPcs.end(), pc) == lowPcs.end())
                        { addsSomething = true; break; }
                    if (! addsSomething)
                        continue;

                    // обе части вместе должны покрыть весь набор без остатка
                    std::set<int> uni (lowPcs.begin(), lowPcs.end());
                    uni.insert (upPcs.begin(), upPcs.end());
                    if (uni.size() != set.size())
                        continue;

                    return naming.name (upRoot) + up.name + " / "
                         + naming.name (r.bassPc) + low.name;
                }
            }
        }
        return {};
    }
}
