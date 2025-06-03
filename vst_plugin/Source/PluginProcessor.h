#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "FootstepClassifier.h"
#include <atomic>

class FootstepDetectorAudioProcessor : public juce::AudioProcessor
{
public:
    FootstepDetectorAudioProcessor();
    ~FootstepDetectorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    // Enhanced parameter range for significant amplification
    float sensitivityParam = 0.5f;
    float gainParam = 3.0f;
    bool bypassParam = false;

private:
    std::unique_ptr<FootstepClassifier> footstepClassifier;
    
    // Enhanced multi-band amplification system (40-500Hz focus)
    std::vector<std::vector<float>> amplificationFilters;
    
    // Thread-safe debug counters
    std::atomic<int> debugCounter{0};
    std::atomic<int> detectionCount{0};
    
    // Advanced amplification methods
    float applyAdvancedAmplification(float inputSample, float gain, int channel);
    float applyAmplificationFilter(float sample, int bandIndex, int channel);
    float applyDynamicCompression(float sample);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessor)
};