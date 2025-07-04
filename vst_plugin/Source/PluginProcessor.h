#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "MLFootstepClassifier.h"  // ONLY ML classifier

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

    float getParameter(int index) override;
    void setParameter(int index, float value) override;
    const juce::String getParameterName(int index) override;
    const juce::String getParameterText(int index) override;
    int getNumParameters() override { return 3; }

    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* sensitivityParam = nullptr;
    std::atomic<float>* enhancementParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

    // SIMPLIFIED: Only ML classifier
    MLFootstepClassifier* getFootstepClassifier() const { return mlFootstepClassifier.get(); }

private:
    // CLEAN: Only ML classifier, no fallback complexity
    std::unique_ptr<MLFootstepClassifier> mlFootstepClassifier;
    
    std::vector<juce::dsp::IIR::Filter<float>> lowShelfFilter;
    std::vector<juce::dsp::IIR::Filter<float>> midShelfFilter;
    std::vector<juce::dsp::IIR::Filter<float>> highShelfFilter;
    
    mutable juce::CriticalSection processingLock;
    bool isProcessing = false;
    
    float currentAmplification = 1.0f;
    float targetAmplification = 1.0f;
    float envelopeAttack = 0.002f;
    float envelopeRelease = 0.0008f;

    int holdSamples = 0;
    int footstepHoldDuration = 0;
    bool inHoldPhase = false;
    
    float applyFootstepEQ(float sample, int channel);
    float applyMultiBandEQ(float sample, int channel);
    void getEditorSize(int& width, int& height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessor)
};