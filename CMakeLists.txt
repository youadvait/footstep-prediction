cmake_minimum_required(VERSION 3.15)
project(FootstepDetector VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Release build by default for Windows
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
    PRODUCT_NAME "FootstepDetector"  # FIXED: Removed "ML" and space
    COMPANY_NAME "FootstepTech"
    PLUGIN_MANUFACTURER_CODE FtTc
    PLUGIN_CODE FsDt
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx"
    BUNDLE_ID "com.footsteptech.footstepdetector"  # FIXED: Removed .ml
    DESCRIPTION "ML-powered footstep detection for gaming"
    PLUGIN_VERSION 1.0.0
)


# UPDATED: Include ML classifier and remove old classifier
target_sources(FootstepDetector PRIVATE
    vst_plugin/Source/PluginProcessor.cpp
    vst_plugin/Source/PluginEditor.cpp
    vst_plugin/Source/MLFootstepClassifier.cpp  # NEW: ML classifier
)

# OPTIONAL: Include model files as resources
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vst_plugin/Source/models/footstep_detector_realistic.tflite")
    target_sources(FootstepDetector PRIVATE
        vst_plugin/Source/models/footstep_detector_realistic.tflite
        vst_plugin/Source/models/deployment_package_realistic.pkl
    )
    
    # Copy model files to build directory
    add_custom_command(TARGET FootstepDetector POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/vst_plugin/Source/models"
        "$<TARGET_FILE_DIR:FootstepDetector>/models"
        COMMENT "Copying ML model files to build directory"
    )
    
    message(STATUS "ML model files will be included in build")
else()
    message(STATUS "ML model files not found - plugin will use pre-trained weights")
endif()

target_link_libraries(FootstepDetector PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_core
    juce::juce_dsp
)

target_compile_definitions(FootstepDetector PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISABLE_ASSERTIONS=1
    JUCE_CHECK_MEMORY_LEAKS=0
    JUCE_STRICT_REFCOUNTEDPOINTER=0
    JUCE_OPENGL=0
    JUCE_DISABLE_MIDI=1
    JUCE_DISABLE_COREMIDI=1
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    
    # ML-specific defines (safe format)
    USE_ML_CLASSIFIER=1
    
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0601>
    $<$<PLATFORM_ID:Windows>:WINVER=0x0601>
    $<$<PLATFORM_ID:Windows>:JUCE_USE_WIN32_FILEBROWSER=1>
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN=1>
    $<$<PLATFORM_ID:Darwin>:JUCE_USE_NATIVE_ALERT_WINDOWS=1>
)

if(WIN32)
    target_link_options(FootstepDetector PRIVATE
        /SUBSYSTEM:WINDOWS
        /MACHINE:X64
    )
    
    set_property(TARGET FootstepDetector PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Print build configuration
message(STATUS "Building FootstepDetector with ML Enhancement")
message(STATUS "ML Classifier: ENABLED")
message(STATUS "Optimized for: Gaming Audio")