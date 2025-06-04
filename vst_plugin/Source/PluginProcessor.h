#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "FootstepClassifier.h"

class FootstepDetectorAudioProcessor : public juce::AudioProcessor
{
public:
    // FIXED: Proper constructor with bus configuration
    FootstepDetectorAudioProcessor();
    ~FootstepDetectorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    
    void getEditorSize(int& width, int& height);

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // FIXED: Use AudioProcessorValueTreeState for proper parameter management
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter pointers
    std::atomic<float>* sensitivityParam = nullptr;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

private:
    std::unique_ptr<FootstepClassifier> footstepClassifier;
    std::vector<juce::dsp::IIR::Filter<float>> lowShelfFilter;
    
    // ENHANCED: Additional frequency bands for full footstep spectrum
    std::vector<juce::dsp::IIR::Filter<float>> midShelfFilter;   // 300Hz band
    std::vector<juce::dsp::IIR::Filter<float>> highShelfFilter;  // 450Hz band
    
    mutable juce::CriticalSection processingLock;
    std::atomic<bool> isProcessing{false};

    float applyFootstepEQ(float sample, int channel);
    float applyMultiBandEQ(float sample, int channel); // ENHANCED: Multi-band processing
    float applySaturation(float sample);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessor)
};