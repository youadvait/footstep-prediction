cmake_minimum_required(VERSION 3.15)
project(FootstepDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(JUCE)

# OPTION 1: Custom Standalone with JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
juce_add_plugin(FootstepDetector
    PRODUCT_NAME "FootstepDetector"
    COMPANY_NAME "FootstepTech"
    PLUGIN_MANUFACTURER_CODE FtTc
    PLUGIN_CODE FsDt
    FORMATS Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(FootstepDetector PRIVATE
    vst_plugin/Source/StandaloneApp.cpp  # Your custom standalone
    vst_plugin/Source/PluginProcessor.cpp
    vst_plugin/Source/PluginEditor.cpp
    vst_plugin/Source/FootstepClassifier.cpp
)

target_link_libraries(FootstepDetector PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_core
)

# CRITICAL: Enable custom standalone (from search result [2])
target_compile_definitions(FootstepDetector PRIVATE
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1  # KEY FIX from search results
    JUCE_DISABLE_MIDI=1
    JUCE_DISABLE_COREMIDI=1
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
)