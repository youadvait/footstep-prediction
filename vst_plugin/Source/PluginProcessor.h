#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "FootstepClassifier.h"  // ← Remove MFCCExtractor include
#include <vector>
#include <array>
#include <memory>

class FootstepDetectorEditor;

class FootstepDetectorProcessor : public juce::AudioProcessor
{
public:
    FootstepDetectorProcessor();
    ~FootstepDetectorProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int /*index*/) override {}
    const juce::String getProgramName (int /*index*/) override { return {}; }
    void changeProgramName (int /*index*/, const juce::String& /*newName*/) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameter access for UI
    float getSensitivity() const { return sensitivity; }
    void setSensitivity(float newSensitivity) { sensitivity = newSensitivity; }
    
    float getFootstepGain() const { return footstepGain; }
    void setFootstepGain(float newGain) { footstepGain = newGain; }
    
    bool getBypassed() const { return bypassed; }
    void setBypassed(bool shouldBypass) { bypassed = shouldBypass; }
    
    float getCurrentConfidence() const { return currentConfidence; }

private:
    // Audio processing parameters
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    // User parameters
    std::atomic<float> sensitivity{0.5f};
    std::atomic<float> footstepGain{3.0f};
    std::atomic<bool> bypassed{false};
    
    // Detection state
    std::atomic<float> currentConfidence{0.0f};
    
    // ML Components - ONLY FootstepClassifier needed
    std::unique_ptr<FootstepClassifier> footstepClassifier;
    
    // Audio analysis buffers (minimal)
    std::vector<float> analysisBuffer;
    int analysisBufferPos = 0;
    static constexpr int ANALYSIS_WINDOW_SIZE = 2048;
    
    // Energy-based detection
    std::vector<float> energyHistory;
    int bufferWritePosition = 0;
    static constexpr int ENERGY_HISTORY_SIZE = 20;
    
    // ONLY these methods needed:
    void performDirectAudioAnalysis(const float* audioData, int numSamples);
    void applyFootstepAmplification(juce::AudioBuffer<float>& buffer, float confidence);
    void initializeBuffers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FootstepDetectorProcessor)
};