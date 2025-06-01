#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class FootstepDetectorEditor : public juce::AudioProcessorEditor
{
public:
    FootstepDetectorEditor (FootstepDetectorProcessor&);
    ~FootstepDetectorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    FootstepDetectorProcessor& audioProcessor;

    // UI Components
    juce::Slider sensitivitySlider;
    juce::Label sensitivityLabel;
    
    juce::Slider gainSlider;
    juce::Label gainLabel;
    
    juce::ToggleButton bypassButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FootstepDetectorEditor)
};