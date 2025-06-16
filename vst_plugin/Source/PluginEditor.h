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
    
    private:
    juce::Slider reductionSlider;    // Was gainSlider
    juce::Label reductionLabel;      // Was gainLabel
    juce::Slider enhancementSlider;  // NEW
    juce::Label enhancementLabel;    // NEW
    
    juce::ToggleButton bypassButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessorEditor)
};
