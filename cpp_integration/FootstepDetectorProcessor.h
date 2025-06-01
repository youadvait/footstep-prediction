
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "FootstepClassifier.h"

class FootstepDetectorProcessor : public juce::AudioProcessor {
public:
    FootstepDetectorProcessor();
    ~FootstepDetectorProcessor();
    
    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    
    // Plugin info
    const juce::String getName() const override { return "Footstep Detector"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    // Programs
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return "Default"; }
    void changeProgramName(int index, const juce::String& newName) override {}
    
    // State
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    // Editor
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    // Parameters
    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    
private:
    // Core components
    FootstepClassifier footstepClassifier;
    
    // Processing buffers
    juce::AudioBuffer<float> processingBuffer;
    std::vector<float> analysisWindow;
    int windowPosition;
    
    // EQ for footstep amplification
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                   juce::dsp::IIR::Coefficients<float>> lowBandFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, 
                                   juce::dsp::IIR::Coefficients<float>> midBandFilter;
    
    // Parameters
    juce::AudioProcessorValueTreeState valueTreeState;
    std::atomic<float>* sensitivityParam;
    std::atomic<float>* lowBandGainParam;
    std::atomic<float>* midBandGainParam;
    std::atomic<float>* bypassParam;
    
    // Real-time detection state
    float currentFootstepConfidence;
    float smoothedConfidence;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorProcessor)
};
