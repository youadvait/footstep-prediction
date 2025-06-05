cmake_minimum_required(VERSION 3.15)
project(FootstepDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Windows-specific optimizations
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /O2")
endif()

add_subdirectory(JUCE)

juce_add_plugin(FootstepDetector
    PRODUCT_NAME "FootstepDetector"
    COMPANY_NAME "FootstepTech"
    PLUGIN_MANUFACTURER_CODE FtTc
    PLUGIN_CODE FsDt
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE  # Manual control for reliability
    VST3_CATEGORIES "Fx"
    BUNDLE_ID "com.footsteptech.footstepdetector"
    DESCRIPTION "Professional footstep detection and enhancement for gaming"
    PLUGIN_VERSION 1.0.0
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
    # Core JUCE settings
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    
    # VST3 settings
    JUCE_VST3_CAN_REPLACE_VST2=0
    
    # Performance and stability
    JUCE_DISABLE_ASSERTIONS=1
    JUCE_CHECK_MEMORY_LEAKS=0
    JUCE_STRICT_REFCOUNTEDPOINTER=0
    JUCE_OPENGL=0
    
    # Platform-specific Windows compatibility
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
    $<$<PLATFORM_ID:Windows>:WINVER=0x0601>
    $<$<PLATFORM_ID:Windows>:JUCE_USE_WIN32_FILEBROWSER=1>
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN=1>
    
    # macOS-specific
    $<$<PLATFORM_ID:Darwin>:JUCE_USE_NATIVE_ALERT_WINDOWS=1>
)

# Windows-specific linking for maximum compatibility
if(WIN32)
    target_link_options(FootstepDetector PRIVATE
        /SUBSYSTEM:WINDOWS
        /MACHINE:X64
    )
    
    # Static runtime linking for standalone distribution
    set_property(TARGET FootstepDetector PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Ensure proper VST3 structure on Windows
if(WIN32)
    # From search result [5] - Windows VST3 should be bundle format
    set_target_properties(FootstepDetector_VST3 PROPERTIES
        SUFFIX ""
        OUTPUT_NAME "FootstepDetector"
    )
endif()
