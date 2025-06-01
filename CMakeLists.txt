cmake_minimum_required(VERSION 3.22)
project(FootstepDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release for consistent builds
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Windows optimizations
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
add_subdirectory(${JUCE_DIR})

# Simple plugin configuration
juce_add_plugin(FootstepDetector
    COMPANY_NAME "FootstepTech"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE "FtTc"
    PLUGIN_CODE "FsDt"
    FORMATS VST3
    PRODUCT_NAME "FootstepDetector"
)

target_sources(FootstepDetector
    PRIVATE
        vst_plugin/Source/PluginProcessor.cpp
        vst_plugin/Source/PluginEditor.cpp
        vst_plugin/Source/FootstepClassifier.cpp
)

# Simple include directories
target_include_directories(FootstepDetector
    PRIVATE
        vst_plugin/Source
)

target_link_libraries(FootstepDetector
    PRIVATE
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
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Simple compile definitions that work on all platforms
target_compile_definitions(FootstepDetector
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_CAMERA=0
        JUCE_USE_MP3AUDIOFORMAT=0
        JUCE_USE_LAME_AUDIO_FORMAT=0
        JUCE_USE_OGGVORBIS=0
        JUCE_USE_FLAC=0
        JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_MODAL_LOOPS_PERMITTED=1
)

# Platform-specific definitions (simple approach)
if(WIN32)
    target_compile_definitions(FootstepDetector
        PRIVATE
            JUCE_USE_WINRT_MIDI=0
            WIN32_LEAN_AND_MEAN=1
            NOMINMAX=1
    )
endif()

if(APPLE)
    target_compile_definitions(FootstepDetector
        PRIVATE
            JUCE_COREMIDI=0
            JUCE_DISABLE_COREMIDI=1
    )
endif()

if(UNIX AND NOT APPLE)
    target_compile_definitions(FootstepDetector
        PRIVATE
            JUCE_ALSA_MIDI=0
            JUCE_JACK_MIDI=0
    )
endif()
