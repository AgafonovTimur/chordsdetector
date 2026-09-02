cmake_minimum_required(VERSION 3.22)

project(NotesToWeb VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- JUCE скачивается автоматически при сборке ----
include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.8
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(JUCE)

# ---- Плагин ----
juce_add_plugin(NotesToWeb
    COMPANY_NAME              "NotesToWeb"
    BUNDLE_ID                 "com.notestoweb.chorddisplay"
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         TRUE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    PLUGIN_MANUFACTURER_CODE  Ntw1
    PLUGIN_CODE               Ntwc
    FORMATS                   VST3
    VST3_CATEGORIES           "Instrument" "Tools"
    PRODUCT_NAME              "Notes To Web")

# ---- Встроенный шрифт ----
juce_add_binary_data(NotesToWebData
    SOURCES Resources/PTSansBold.ttf)

target_sources(NotesToWeb PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(NotesToWeb PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(NotesToWeb
    PRIVATE
        NotesToWebData
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Исходники в UTF-8 (иначе MSVC ломает русский текст в интерфейсе)
if(MSVC)
    target_compile_options(NotesToWeb PRIVATE /utf-8)
endif()
