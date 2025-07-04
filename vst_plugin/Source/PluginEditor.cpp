#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessorEditor::FootstepDetectorAudioProcessorEditor(FootstepDetectorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Sensitivity slider
    sensitivitySlider.setRange(0.0, 1.0, 0.01);
    sensitivitySlider.setValue(audioProcessor.sensitivityParam->load());
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    sensitivitySlider.onValueChange = [this]()
    {
        float newValue = static_cast<float>(sensitivitySlider.getValue());
        newValue = juce::jlimit(0.0f, 1.0f, newValue);
        audioProcessor.sensitivityParam->store(newValue);
    };
    addAndMakeVisible(sensitivitySlider);
    
    sensitivityLabel.setText("Sensitivity", juce::dontSendNotification);
    sensitivityLabel.attachToComponent(&sensitivitySlider, true);
    addAndMakeVisible(sensitivityLabel);
    
    // Enhancement slider (updated range to match processor)
    enhancementSlider.setRange(1.0, 1.4, 0.01); // FIXED: Match processor range
    enhancementSlider.setValue(audioProcessor.enhancementParam->load());
    enhancementSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    enhancementSlider.onValueChange = [this]()
    {
        float newValue = static_cast<float>(enhancementSlider.getValue());
        newValue = juce::jlimit(1.0f, 1.4f, newValue); // FIXED: Match processor range
        audioProcessor.enhancementParam->store(newValue);
    };
    addAndMakeVisible(enhancementSlider);
    
    enhancementLabel.setText("Enhancement", juce::dontSendNotification);
    enhancementLabel.attachToComponent(&enhancementSlider, true);
    addAndMakeVisible(enhancementLabel);
    
    // Bypass button
    bypassButton.setButtonText("Bypass");
    bypassButton.setToggleState(audioProcessor.bypassParam->load() > 0.5f, juce::dontSendNotification);
    bypassButton.onStateChange = [this]()
    {
        audioProcessor.bypassParam->store(bypassButton.getToggleState() ? 1.0f : 0.0f);
    };
    addAndMakeVisible(bypassButton);
    
    setSize(400, 300); // SMALLER: Only 2 sliders now
}

FootstepDetectorAudioProcessorEditor::~FootstepDetectorAudioProcessorEditor()
{
}

void FootstepDetectorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(80); // Space for title
    
    auto sliderArea = area.removeFromTop(80);
    sensitivitySlider.setBounds(sliderArea.removeFromLeft(180).reduced(10));
    enhancementSlider.setBounds(sliderArea.removeFromLeft(180).reduced(10)); // ONLY 2 sliders now
    
    bypassButton.setBounds(area.removeFromTop(40).reduced(20));
}

void FootstepDetectorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("FootstepDetector - ML Enhanced", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);

    bool isDetecting = audioProcessor.getFootstepClassifier() && 
                      !audioProcessor.getFootstepClassifier()->isInCooldown();
    
    if (isDetecting) {
        g.setColour(juce::Colours::green);
        g.fillEllipse(getWidth() - 30, 10, 20, 20);
        g.setColour(juce::Colours::white);
        g.drawText("ENHANCING", getWidth() - 100, 35, 80, 20, juce::Justification::centred);
    } else {
        g.setColour(juce::Colours::grey);
        g.fillEllipse(getWidth() - 30, 10, 20, 20);
        g.setColour(juce::Colours::white);
        g.drawText("MONITORING", getWidth() - 100, 35, 80, 20, juce::Justification::centred);
    }
}
