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
    int getNumParameters() override { return 4; }

    // AudioProcessorValueTreeState for proper parameter management
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter pointers
    std::atomic<float>* sensitivityParam = nullptr;
    std::atomic<float>* reductionParam = nullptr;    // How much to reduce non-footsteps
    std::atomic<float>* enhancementParam = nullptr;  // How much to enhance footsteps
    std::atomic<float>* bypassParam = nullptr;

private:
    std::unique_ptr<FootstepClassifier> footstepClassifier;
    std::vector<juce::dsp::IIR::Filter<float>> lowShelfFilter;
    std::vector<juce::dsp::IIR::Filter<float>> midShelfFilter;
    std::vector<juce::dsp::IIR::Filter<float>> highShelfFilter;
    
    // ADD: Smooth envelope for crackling-free amplification
    mutable juce::CriticalSection processingLock;
    bool isProcessing = false;
    
    // NEW: Smooth amplification envelope
    float currentAmplification = 1.0f;
    float targetAmplification = 1.0f;
    float envelopeAttack = 0.0045f;   // Very fast attack
    float envelopeRelease = 0.0003f;   // Smooth release

    int holdSamples = 0;             // Remaining hold time in samples
    int footstepHoldDuration = 0;    // Total hold duration (200ms)
    bool inHoldPhase = false;        // Currently holding amplification
    
    float applyFootstepEQ(float sample, int channel);
    float applyMultiBandEQ(float sample, int channel);
    void getEditorSize(int& width, int& height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessor)
};
