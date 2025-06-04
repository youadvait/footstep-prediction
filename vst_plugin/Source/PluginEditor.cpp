#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessorEditor::FootstepDetectorAudioProcessorEditor(FootstepDetectorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Sensitivity slider
    sensitivitySlider.setRange(0.0, 1.0, 0.01);
    // FIXED: Load value from atomic parameter pointer
    sensitivitySlider.setValue(audioProcessor.sensitivityParam->load());
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    sensitivitySlider.onValueChange = [this]()
    {
        float newValue = static_cast<float>(sensitivitySlider.getValue());
        newValue = juce::jlimit(0.0f, 1.0f, newValue);
        // FIXED: Store value to atomic parameter pointer
        audioProcessor.sensitivityParam->store(newValue);
    };
    addAndMakeVisible(sensitivitySlider);
    
    sensitivityLabel.setText("Sensitivity", juce::dontSendNotification);
    sensitivityLabel.attachToComponent(&sensitivitySlider, true);
    addAndMakeVisible(sensitivityLabel);
    
    // Gain slider
    gainSlider.setRange(1.0, 8.0, 0.1);
    // FIXED: Load value from atomic parameter pointer
    gainSlider.setValue(audioProcessor.gainParam->load());
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.onValueChange = [this]()
    {
        float newValue = static_cast<float>(gainSlider.getValue());
        newValue = juce::jlimit(1.0f, 8.0f, newValue);
        // FIXED: Store value to atomic parameter pointer
        audioProcessor.gainParam->store(newValue);
    };
    addAndMakeVisible(gainSlider);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, true);
    addAndMakeVisible(gainLabel);
    
    // Bypass button
    bypassButton.setButtonText("Bypass");
    // FIXED: Load value from atomic parameter pointer
    bypassButton.setToggleState(audioProcessor.bypassParam->load() > 0.5f, juce::dontSendNotification);
    bypassButton.onStateChange = [this]()
    {
        // FIXED: Store boolean as float to atomic parameter pointer
        audioProcessor.bypassParam->store(bypassButton.getToggleState() ? 1.0f : 0.0f);
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