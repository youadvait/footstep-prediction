#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class FootstepDetectorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FootstepDetectorAudioProcessorEditor(FootstepDetectorAudioProcessor& p);
    ~FootstepDetectorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Reference to processor
    FootstepDetectorAudioProcessor& audioProcessor;

    // UI components
    juce::Slider sensitivitySlider;
    juce::Label sensitivityLabel;
    
    juce::Slider gainSlider;
    juce::Label gainLabel;
    
    juce::ToggleButton bypassButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessorEditor)
};
