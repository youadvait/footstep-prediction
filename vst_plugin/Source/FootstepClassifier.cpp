#include "FootstepClassifier.h"
#include <algorithm>
#include <iostream>

FootstepClassifier::FootstepClassifier()
{
    // Initialize buffers for analysis
    energyBuffer.resize(256, 0.0f);
    frequencyBuffer.resize(128, 0.0f);
}

FootstepClassifier::~FootstepClassifier()
{
}

void FootstepClassifier::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Reset analysis state
    std::fill(energyBuffer.begin(), energyBuffer.end(), 0.0f);
    std::fill(frequencyBuffer.begin(), frequencyBuffer.end(), 0.0f);
    energyBufferPos = 0;
    frequencyBufferPos = 0;
    currentEnergy = 0.0f;
    lastConfidence = 0.0f;
    lastEnergy = 0.0f;
    cooldownCounter = 0;
}

bool FootstepClassifier::detectFootstep(float inputSample, float sensitivity)
{
    // Existing validation...
    
    float energy = calculateEnergy(inputSample);
    float frequency = calculateFrequencyContent(inputSample);
    float confidence = calculateConfidence(energy, frequency);
    
    // MUCH HIGHER threshold to reduce false positives
    float threshold = 0.8f + (1.0f - sensitivity) * 0.15f; // Range: 0.65 to 0.8 (was 0.3-0.7!)
    
    // LONGER cooldown to prevent rapid triggering
    if (cooldownCounter > 0)
    {
        cooldownCounter--;
        return false;
    }
    
    bool isFootstep = confidence > threshold;
    
    if (isFootstep)
    {
        // MUCH LONGER cooldown - 300ms instead of 100ms
        cooldownCounter = static_cast<int>(currentSampleRate * 0.3); 
        
        std::cout << "🎮 FOOTSTEP DETECTED | Confidence: " << confidence 
                  << " | Threshold: " << threshold << std::endl;
    }
    
    return isFootstep;
}

float FootstepClassifier::calculateEnergy(float sample)
{
    // RMS energy calculation
    float squaredSample = sample * sample;
    
    // Update energy buffer
    energyBuffer[energyBufferPos] = squaredSample;
    energyBufferPos = (energyBufferPos + 1) % energyBuffer.size();
    
    // Calculate RMS over buffer
    float sum = 0.0f;
    for (float value : energyBuffer)
    {
        sum += value;
    }
    
    currentEnergy = std::sqrt(sum / energyBuffer.size());
    return currentEnergy;
}

float FootstepClassifier::calculateFrequencyContent(float sample)
{
    // Simple high-pass filter for footstep frequency range
    static float lastSample = 0.0f;
    float highPass = sample - lastSample;
    lastSample = sample;
    
    // Update frequency buffer
    frequencyBuffer[frequencyBufferPos] = std::abs(highPass);
    frequencyBufferPos = (frequencyBufferPos + 1) % frequencyBuffer.size();
    
    // Calculate average frequency content
    float sum = 0.0f;
    for (float value : frequencyBuffer)
    {
        sum += value;
    }
    
    return sum / frequencyBuffer.size();
}

float FootstepClassifier::calculateConfidence(float energy, float frequency)
{
    // Footstep detection algorithm
    // Energy threshold: typical footsteps have RMS between 0.025 and 0.100
    float energyScore = 0.0f;
    if (energy >= 0.025f && energy <= 0.100f)
    {
        energyScore = 1.0f - std::abs(0.0625f - energy) / 0.0375f; // Peak at 0.0625
    }
    
    // Frequency content: footsteps have mid-frequency content
    float frequencyScore = std::min(1.0f, frequency * 10.0f); // Scale frequency content
    
    // Combined confidence
    float confidence = (energyScore * 0.7f) + (frequencyScore * 0.3f);
    
    return std::max(0.0f, std::min(1.0f, confidence));
}

// ADD GETTER METHODS for compatibility
float FootstepClassifier::getLastConfidence() const 
{ 
    return lastConfidence; 
}

float FootstepClassifier::getLastEnergy() const 
{ 
    return lastEnergy; 
}

float FootstepClassifier::getBackgroundNoise() const 
{ 
    return 0.01f; // Simple default
}

bool FootstepClassifier::isInCooldown() const 
{ 
    return cooldownCounter > 0; 
}