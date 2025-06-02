#pragma once

#include <JuceHeader.h>
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

    // Simple parameter members (thread-safe primitives)
    std::atomic<float> sensitivityParam{0.5f};
    std::atomic<float> gainParam{3.0f};
    std::atomic<bool> bypassParam{false};

private:
    std::unique_ptr<FootstepClassifier> footstepClassifier;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FootstepDetectorAudioProcessor)
};
