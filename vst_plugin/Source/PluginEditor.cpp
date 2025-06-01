#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorEditor::FootstepDetectorEditor (FootstepDetectorProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Initialize UI components safely
    sensitivitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    sensitivitySlider.setRange(0.0, 1.0, 0.01);
    sensitivitySlider.setValue(0.5);
    sensitivitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    sensitivitySlider.onValueChange = [this] { 
        audioProcessor.setSensitivity(static_cast<float>(sensitivitySlider.getValue())); 
    };
    addAndMakeVisible(sensitivitySlider);
    
    sensitivityLabel.setText("Sensitivity", juce::dontSendNotification);
    sensitivityLabel.attachToComponent(&sensitivitySlider, true);
    addAndMakeVisible(sensitivityLabel);
    
    gainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    gainSlider.setRange(1.0, 10.0, 0.1);
    gainSlider.setValue(3.0);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    gainSlider.onValueChange = [this] { 
        audioProcessor.setFootstepGain(static_cast<float>(gainSlider.getValue())); 
    };
    addAndMakeVisible(gainSlider);
    
    gainLabel.setText("Footstep Gain", juce::dontSendNotification);
    gainLabel.attachToComponent(&gainSlider, true);
    addAndMakeVisible(gainLabel);
    
    bypassButton.setButtonText("Bypass");
    addAndMakeVisible(bypassButton);

    // NO TIMER - avoid string formatting issues completely
    setSize(400, 150);
}

FootstepDetectorEditor::~FootstepDetectorEditor()
{
}

void FootstepDetectorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(40, 42, 54));
    
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("🎮 COD Footstep Detector", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
    
    // SIMPLE static confidence display (no dynamic updates to avoid string issues)
    auto confidenceArea = getLocalBounds().removeFromBottom(40).reduced(20, 10);
    
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(confidenceArea, 2);
    
    g.setColour(juce::Colours::white);
    g.drawFittedText("Confidence: Check Console", confidenceArea, juce::Justification::centred, 1);
}

void FootstepDetectorEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop(35);
    area.removeFromBottom(50);
    
    auto sliderHeight = 35;
    auto margin = 10;
    
    auto sensitivityArea = area.removeFromTop(sliderHeight);
    sensitivityArea.removeFromLeft(100);
    sensitivitySlider.setBounds(sensitivityArea.reduced(margin, 5));
    
    area.removeFromTop(margin);
    
    auto gainArea = area.removeFromTop(sliderHeight);
    gainArea.removeFromLeft(100);
    gainSlider.setBounds(gainArea.reduced(margin, 5));
    
    area.removeFromTop(margin);
    
    bypassButton.setBounds(area.removeFromTop(30).reduced(margin, 0));
}