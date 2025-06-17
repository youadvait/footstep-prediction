#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessorEditor::FootstepDetectorAudioProcessorEditor(FootstepDetectorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Sensitivity slider (unchanged)
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
    
    // UPDATED: Reduction slider (was gain slider)
    reductionSlider.setRange(0.1, 0.8, 0.01);
    reductionSlider.setValue(audioProcessor.reductionParam->load());
    reductionSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    reductionSlider.onValueChange = [this]()
    {
        float newValue = static_cast<float>(reductionSlider.getValue());
        newValue = juce::jlimit(0.1f, 0.8f, newValue);
        audioProcessor.reductionParam->store(newValue);
    };
    addAndMakeVisible(reductionSlider);
    
    reductionLabel.setText("Reduction", juce::dontSendNotification);
    reductionLabel.attachToComponent(&reductionSlider, true);
    addAndMakeVisible(reductionLabel);
    
    // NEW: Enhancement slider
    enhancementSlider.setRange(1.0, 2.0, 0.1);
    enhancementSlider.setValue(audioProcessor.enhancementParam->load());
    enhancementSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    enhancementSlider.onValueChange = [this]()
    {
        float newValue = static_cast<float>(enhancementSlider.getValue());
        newValue = juce::jlimit(1.0f, 2.0f, newValue);
        audioProcessor.enhancementParam->store(newValue);
    };
    addAndMakeVisible(enhancementSlider);
    
    enhancementLabel.setText("Enhancement", juce::dontSendNotification);
    enhancementLabel.attachToComponent(&enhancementSlider, true);
    addAndMakeVisible(enhancementLabel);
    
    // Bypass button (unchanged)
    bypassButton.setButtonText("Bypass");
    bypassButton.setToggleState(audioProcessor.bypassParam->load() > 0.5f, juce::dontSendNotification);
    bypassButton.onStateChange = [this]()
    {
        audioProcessor.bypassParam->store(bypassButton.getToggleState() ? 1.0f : 0.0f);
    };
    addAndMakeVisible(bypassButton);
    
    setSize(500, 400); // Larger size for 3 sliders
}

FootstepDetectorAudioProcessorEditor::~FootstepDetectorAudioProcessorEditor()
{
}

void FootstepDetectorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(100); // Space for title
    
    auto sliderArea = area.removeFromTop(80);
    sensitivitySlider.setBounds(sliderArea.removeFromLeft(150).reduced(10));
    reductionSlider.setBounds(sliderArea.removeFromLeft(150).reduced(10));    // Was gainSlider
    enhancementSlider.setBounds(sliderArea.removeFromLeft(150).reduced(10));  // NEW
    
    bypassButton.setBounds(area.removeFromTop(40).reduced(20));
}

void FootstepDetectorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("FootstepDetector - Noise Gate", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);

    bool isDetecting = audioProcessor.getFootstepClassifier() && 
                      !audioProcessor.getFootstepClassifier()->isInCooldown();
    
    if (isDetecting) {
        g.setColour(juce::Colours::green);
        g.fillEllipse(getWidth() - 30, 10, 20, 20);
        g.setColour(juce::Colours::white);
        g.drawText("DETECTING", getWidth() - 100, 35, 80, 20, juce::Justification::centred);
    } else {
        g.setColour(juce::Colours::red);
        g.fillEllipse(getWidth() - 30, 10, 20, 20);
        g.setColour(juce::Colours::white);
        g.drawText("REDUCING", getWidth() - 100, 35, 80, 20, juce::Justification::centred);
    }
}
