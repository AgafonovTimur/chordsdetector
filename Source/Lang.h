#pragma once

#include <juce_core/juce_core.h>

//==============================================================================
//  Тексты интерфейса на трёх языках.
//
//  ВАЖНО: juce::String, созданная из обычного const char*, считает байты
//  однобайтовой кодировкой и портит любые нелатинские буквы. Поэтому весь
//  текст обязательно оборачивается в juce::CharPointer_UTF8 — см. utf8() ниже.
//==============================================================================
namespace Lang
{
    enum Id { Russian = 0, English = 1, Chinese = 2 };

    inline juce::String utf8 (const char* s)
    {
        return juce::String (juce::CharPointer_UTF8 (s));
    }

    inline juce::String tr (int lang, const char* ru, const char* en, const char* zh)
    {
        switch (lang)
        {
            case English: return utf8 (en);
            case Chinese: return utf8 (zh);
            default:      return utf8 (ru);
        }
    }

    // ---- Названия языков для кнопок выбора ----
    inline juce::String languageName (int lang)
    {
        switch (lang)
        {
            case English: return utf8 ("English");
            case Chinese: return utf8 ("\xe4\xb8\xad\xe6\x96\x87");
            default:      return utf8 ("\xd0\xa0\xd1\x83\xd1\x81\xd1\x81\xd0\xba\xd0\xb8\xd0\xb9");
        }
    }

    // ---- Строки интерфейса ----
    inline juce::String settings (int l)      { return tr (l, "Настройки", "Settings", "设置"); }
    inline juce::String pressANote (int l)    { return tr (l, "нажмите ноту", "play a note", "请弹一个音"); }

    inline juce::String secLanguage (int l)   { return tr (l, "ЯЗЫК", "LANGUAGE", "语言"); }
    inline juce::String secGeneral (int l)    { return tr (l, "ОБЩЕЕ", "GENERAL", "常规"); }
    inline juce::String secChordLine (int l)  { return tr (l, "СТРОКА АККОРДА", "CHORD LINE", "和弦显示"); }
    inline juce::String secColours (int l)    { return tr (l, "ЦВЕТА", "COLOURS", "颜色"); }
    inline juce::String secFlats (int l)      { return tr (l, "БЕМОЛЬ ВМЕСТО ДИЕЗА", "FLATS INSTEAD OF SHARPS", "用降号代替升号"); }
    inline juce::String secTonic (int l)      { return tr (l, "ТОНИКА (ТОЛЬКО ОДНА)", "TONIC (ONE ONLY)", "主音（只能选一个）"); }
    inline juce::String secMode (int l)       { return tr (l, "ЛАД (ТОЛЬКО ОДИН)", "SCALE (ONE ONLY)", "调式（只能选一个）"); }
    inline juce::String secInversion (int l)  { return tr (l, "НОМЕР ОБРАЩЕНИЯ", "INVERSION NUMBER", "转位编号"); }
    inline juce::String secDegree (int l)     { return tr (l, "СТУПЕНЬ ЛАДА", "SCALE DEGREE", "音级"); }
    inline juce::String secPoly (int l)       { return tr (l, "ПОЛИАККОРД", "POLYCHORD", "复合和弦"); }

    inline juce::String midiThru (int l)      { return tr (l, "Пропускать MIDI дальше (MIDI Thru)", "Pass MIDI through (MIDI Thru)", "转发 MIDI（MIDI Thru）"); }
    inline juce::String captureWindow (int l) { return tr (l, "Окно захвата аккорда, мс", "Chord capture window, ms", "和弦采集窗口（毫秒）"); }
    inline juce::String fitToWindow (int l)   { return tr (l, "Подгонять аккорд под ширину окна", "Fit chord to window width", "和弦自适应窗口宽度"); }
    inline juce::String chordSize (int l)     { return tr (l, "Размер аккорда, %", "Chord size, %", "和弦大小 %"); }

    inline juce::String bgColour (int l)      { return tr (l, "Цвет фона", "Background colour", "背景颜色"); }
    inline juce::String textColour (int l)    { return tr (l, "Цвет текста", "Text colour", "文字颜色"); }
    inline juce::String pickColour (int l)    { return tr (l, "Выбрать", "Choose", "选择"); }

    inline juce::String showInversion (int l) { return tr (l, "Показывать номер обращения", "Show inversion number", "显示转位编号"); }
    inline juce::String showDegree (int l)    { return tr (l, "Показывать ступень лада", "Show scale degree", "显示音级"); }
    inline juce::String showPoly (int l)      { return tr (l, "Показывать полиаккорд, если аккорд не определился",
                                                              "Show polychord when the chord is not recognised",
                                                              "未识别出和弦时显示复合和弦"); }

    inline juce::String size (int l)          { return tr (l, "Размер, %", "Size, %", "大小 %"); }
    inline juce::String posX (int l)          { return tr (l, "Положение по горизонтали, %", "Horizontal position, %", "水平位置 %"); }
    inline juce::String posY (int l)          { return tr (l, "Положение по вертикали, %", "Vertical position, %", "垂直位置 %"); }

    inline juce::String resetDefaults (int l) { return tr (l, "По умолчанию", "Restore defaults", "恢复默认设置"); }

    // ---- Названия ладов ----
    inline juce::String modeName (int l, const juce::String& key)
    {
        if (key == "major")    return tr (l, "Мажор",          "Major",            "大调");
        if (key == "natmin")   return tr (l, "Натур. минор",   "Natural minor",    "自然小调");
        if (key == "harmin")   return tr (l, "Гарм. минор",    "Harmonic minor",   "和声小调");
        if (key == "melmin")   return tr (l, "Мелод. минор",   "Melodic minor",    "旋律小调");
        if (key == "dorian")   return tr (l, "Дорийский",      "Dorian",           "多利亚");
        if (key == "phrygian") return tr (l, "Фригийский",     "Phrygian",         "弗里几亚");
        if (key == "lydian")   return tr (l, "Лидийский",      "Lydian",           "利底亚");
        if (key == "mixo")     return tr (l, "Миксолидийский", "Mixolydian",       "混合利底亚");
        if (key == "locrian")  return tr (l, "Локрийский",     "Locrian",          "洛克里亚");
        return key;
    }
}
