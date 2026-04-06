cmake_minimum_required(VERSION 3.22)
project(DelClip VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Statically link the MSVC runtime so the plugin has no external DLL dependencies
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ── JUCE ──────────────────────────────────────────────────────────────────────
# Add JUCE as a subdirectory (git submodule recommended):
#   git submodule add https://github.com/juce-framework/JUCE.git JUCE
# Or set JUCE_PATH to an existing JUCE install and use find_package.
add_subdirectory(JUCE)

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(DelClip
    VERSION                     "1.0.0"
    COMPANY_NAME                "YourCompany"
    PLUGIN_MANUFACTURER_CODE    "Dlcp"
    PLUGIN_CODE                 "DlCp"
    FORMATS                     VST3 AU Standalone
    PRODUCT_NAME                "DelClip"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(DelClip PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(DelClip PRIVATE Source)

target_compile_definitions(DelClip
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

juce_generate_juce_header(DelClip)

target_link_libraries(DelClip
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
