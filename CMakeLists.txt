cmake_minimum_required(VERSION 3.22)
project(FootstepDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FIXED: Default to Release for Windows, but allow override
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)  # Changed from Debug to Release
endif()

# Windows-specific optimizations
if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
add_subdirectory(${JUCE_DIR})

juce_add_plugin(FootstepDetector
    COMPANY_NAME "FootstepTech"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "FtTc"
    PLUGIN_CODE "FsDt"
    # FIXED: Platform-specific formats
    FORMATS $<$<PLATFORM_ID:Darwin>:Standalone;AU;VST3>$<$<PLATFORM_ID:Windows>:VST3>$<$<PLATFORM_ID:Linux>:VST3>
    PRODUCT_NAME "FootstepDetector"
)

target_sources(FootstepDetector
    PRIVATE
        vst_plugin/Source/PluginProcessor.cpp
        vst_plugin/Source/PluginEditor.cpp
        vst_plugin/Source/FootstepClassifier.cpp
)

# FIXED: Only include directories that exist
target_include_directories(FootstepDetector
    PRIVATE
        vst_plugin/Source
        # Only include these if they exist (Mac-specific)
        $<$<BOOL:${CMAKE_SOURCE_DIR}/cpp_integration>:cpp_integration>
        $<$<BOOL:${CMAKE_SOURCE_DIR}/models>:models>
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
        # Platform-specific MIDI disabling
        $<$<PLATFORM_ID:Darwin>:JUCE_COREMIDI=0>
        $<$<PLATFORM_ID:Darwin>:JUCE_DISABLE_COREMIDI=1>
        $<$<PLATFORM_ID:Windows>:JUCE_USE_WINRT_MIDI=0>
        $<$<PLATFORM_ID:Linux>:JUCE_ALSA_MIDI=0>
        $<$<PLATFORM_ID:Linux>:JUCE_JACK_MIDI=0>
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        # Additional MIDI disabling
        JUCE_MIDI_ENABLED=0
        JUCE_STANDALONE_APPLICATION_MIDI_ENABLED=0
        # Windows-specific
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN=1>
        $<$<PLATFORM_ID:Windows>:NOMINMAX=1>
)

# Windows-specific compiler options
if(MSVC)
    target_compile_options(FootstepDetector PRIVATE /W3)
endif()
