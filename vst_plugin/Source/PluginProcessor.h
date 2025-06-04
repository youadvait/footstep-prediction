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
    FootstepDetectorAudioProcessor();
    ~FootstepDetectorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // FIXED: Only declare these ONCE
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

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

    // EqualizerAPO-compatible parameter methods
    float getParameter(int index) override;
    void setParameter(int index, float value) override;
    const juce::String getParameterName(int index) override;
    const juce::String getParameterText(int index) override;
    int getNumParameters() override { return 3; }

    // AudioProcessorValueTreeState for proper parameter management
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter pointers
    std::atomic<float>* sensitivityParam = nullptr;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

private:
    std::unique_ptr<FootstepClassifier> footstepClassifier;
    std::vector<juce::dsp::IIR::Filter<float>> lowShelfFilter;
    std::vector<juce::dsp::IIR::Filter<float>> midShelfFilter;
    std::vector<juce::dsp::IIR::Filter<float>> highShelfFilter;
    
    // FIXED: Thread safety for EqualizerAPO (proper initialization)
    mutable juce::CriticalSection processingLock;
    
    // FIXED: Use regular bool instead of atomic to avoid copy constructor issues
    bool isProcessing = false;
    
    float applyFootstepEQ(float sample, int channel);
    float applyMultiBandEQ(float sample, int channel);
    float applySaturation(float sample);
    
    // EqualizerAPO compatibility methods
    void getEditorSize(int& width, int& height);

    // FIXED: Prevent copying due to atomic members (from search result [4])
    FootstepDetectorAudioProcessor(const FootstepDetectorAudioProcessor&) = delete;
    FootstepDetectorAudioProcessor& operator=(const FootstepDetectorAudioProcessor&) = delete;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessor)
};
