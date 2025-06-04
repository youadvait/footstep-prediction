#pragma once

#include <vector>
#include <cmath>

class FootstepClassifier
{
public:
    FootstepClassifier();
    ~FootstepClassifier();
    
    // Methods that the processor expects
    void prepare(double sampleRate, int samplesPerBlock);
    bool detectFootstep(float inputSample, float sensitivity);
    
    // ADD: Getter methods for compatibility
    float getLastConfidence() const;
    float getLastEnergy() const;
    float getBackgroundNoise() const;
    bool isInCooldown() const;
    
private:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    // Energy analysis
    std::vector<float> energyBuffer;
    size_t energyBufferPos = 0;
    float currentEnergy = 0.0f;
    
    // Frequency analysis
    std::vector<float> frequencyBuffer;
    size_t frequencyBufferPos = 0;
    
    // Detection state
    float lastConfidence = 0.0f;
    float lastEnergy = 0.0f;
    int cooldownCounter = 0;
    
    // Helper methods
    float calculateEnergy(float sample);
    float calculateFrequencyContent(float sample);
    float calculateConfidence(float energy, float frequency);
};