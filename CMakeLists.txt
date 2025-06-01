cmake_minimum_required(VERSION 3.15)
project(FootstepDetector)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)

juce_add_plugin(FootstepDetector
    PRODUCT_NAME "FootstepDetector"
    COMPANY_NAME "FootstepTech"
    PLUGIN_MANUFACTURER_CODE FtTc
    PLUGIN_CODE FsDt
    FORMATS VST3
    IS_SYNTH FALSE
)

juce_generate_juce_header(FootstepDetector)

target_sources(FootstepDetector PRIVATE
    vst_plugin/Source/PluginProcessor.cpp
    vst_plugin/Source/PluginEditor.cpp
)

target_link_libraries(FootstepDetector PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(FootstepDetector PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
