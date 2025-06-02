#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessorEditor::FootstepDetectorAudioProcessorEditor(FootstepDetectorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Sensitivity slider
    sensitivitySlider.setRange(0.0, 1.0, 0.01);
    sensitivitySlider.setValue(audioProcessor.sensitivityParam);
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    sensitivitySlider.onValueChange = [this]()
    {
        // CRASH PREVENTION: Safe parameter update
        float newValue = static_cast<float>(sensitivitySlider.getValue());
        newValue = juce::jlimit(0.0f, 1.0f, newValue);
        audioProcessor.sensitivityParam = newValue;
    };
    addAndMakeVisible(sensitivitySlider);
    
    sensitivityLabel.setText("Sensitivity", juce::dontSendNotification);
    sensitivityLabel.attachToComponent(&sensitivitySlider, true);
    addAndMakeVisible(sensitivityLabel);
    
    // Gain slider
    gainSlider.setRange(1.0, 5.0, 0.1);
    gainSlider.setValue(audioProcessor.gainParam);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.onValueChange = [this]()
    {
        // CRASH PREVENTION: Safe parameter update
        float newValue = static_cast<float>(gainSlider.getValue());
        newValue = juce::jlimit(1.0f, 5.0f, newValue);
        audioProcessor.gainParam = newValue;
    };
    addAndMakeVisible(gainSlider);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, true);
    addAndMakeVisible(gainLabel);
    
    // Bypass button
    bypassButton.setButtonText("Bypass");
    bypassButton.setToggleState(audioProcessor.bypassParam, juce::dontSendNotification);
    bypassButton.onStateChange = [this]()
    {
        audioProcessor.bypassParam = bypassButton.getToggleState();
    };
    addAndMakeVisible(bypassButton);
    
    setSize(400, 300);
}

FootstepDetectorAudioProcessorEditor::~FootstepDetectorAudioProcessorEditor()
{
}

void FootstepDetectorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("FootstepDetector", getLocalBounds().removeFromTop(60), juce::Justification::centred, 1);
    
    g.setFont(14.0f);
    g.drawFittedText("Call of Duty Footstep Enhancement", getLocalBounds().removeFromTop(100).removeFromBottom(40), juce::Justification::centred, 1);
}

void FootstepDetectorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(100); // Space for title
    
    auto sliderArea = area.removeFromTop(80);
    sensitivitySlider.setBounds(sliderArea.removeFromLeft(180).reduced(10));
    gainSlider.setBounds(sliderArea.removeFromLeft(180).reduced(10));
    
    bypassButton.setBounds(area.removeFromTop(40).reduced(20));
}
