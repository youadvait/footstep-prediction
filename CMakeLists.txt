cmake_minimum_required(VERSION 3.15)
project(FootstepDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Default to Release for Windows
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set VST2 SDK path if provided
if(DEFINED VST2_SDK_PATH)
    message(STATUS "VST2 SDK path set to: ${VST2_SDK_PATH}")
endif()

add_subdirectory(JUCE)

# Set VST2 SDK path for JUCE before adding plugin
if(DEFINED VST2_SDK_PATH)
    juce_set_vst2_sdk_path("${VST2_SDK_PATH}")
endif()

juce_add_plugin(FootstepDetector
    PRODUCT_NAME "FootstepDetector"
    COMPANY_NAME "FootstepTech"
    PLUGIN_MANUFACTURER_CODE FtTc
    PLUGIN_CODE FsDt
    FORMATS VST   # VST2 format for EqualizerAPO
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

target_sources(FootstepDetector PRIVATE
    vst_plugin/Source/PluginProcessor.cpp
    vst_plugin/Source/PluginEditor.cpp
    vst_plugin/Source/FootstepClassifier.cpp
)

target_link_libraries(FootstepDetector PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(FootstepDetector PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    # VST2 specific settings
    JucePlugin_Build_VST=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)
