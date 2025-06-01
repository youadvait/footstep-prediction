cmake_minimum_required(VERSION 3.15)
project(FootstepDetector)

set(CMAKE_CXX_STANDARD 17)

# Find JUCE
find_package(PkgConfig REQUIRED)
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")

if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}")
endif()

add_subdirectory(${JUCE_DIR})

# Create plugin with minimal options
juce_add_plugin(FootstepDetector
    PRODUCT_NAME "FootstepDetector"
    COMPANY_NAME "FootstepTech" 
    PLUGIN_MANUFACTURER_CODE "FtTc"
    PLUGIN_CODE "FsDt"
    FORMATS VST3
    IS_SYNTH FALSE
)

# Add sources
target_sources(FootstepDetector PRIVATE
    vst_plugin/Source/PluginProcessor.cpp
    vst_plugin/Source/PluginEditor.cpp  
    vst_plugin/Source/FootstepClassifier.cpp
)

# Link JUCE
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

# Minimal definitions
target_compile_definitions(FootstepDetector PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
