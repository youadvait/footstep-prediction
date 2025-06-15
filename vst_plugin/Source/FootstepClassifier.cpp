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
    
    // ADAPTIVE: Start with conservative background estimate
    backgroundNoise = 0.015f; // Slightly higher default
}

bool FootstepClassifier::detectFootstep(float inputSample, float sensitivity)
{
    float energy = calculateEnergy(inputSample);
    float frequency = calculateFrequencyContent(inputSample);
    float confidence = calculateConfidence(energy, frequency);
    
    // MUCH LOWER, more sensitive threshold
    float threshold = 0.3f + (1.0f - sensitivity) * 0.4f; // Range: 0.3 to 0.7 (vs 0.7-0.9)
    
    // SHORTER cooldown for responsiveness
    if (cooldownCounter > 0)
    {
        cooldownCounter--;
        return false;
    }
    
    bool isFootstep = confidence > threshold;
    
    if (isFootstep)
    {
        // SHORTER cooldown - 80ms for more responsiveness
        cooldownCounter = static_cast<int>(currentSampleRate * 0.08);
        
        std::cout << "👟 FOOTSTEP | Confidence: " << confidence 
                  << " | Threshold: " << threshold 
                  << " | Energy: " << energy << std::endl;
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
    // IMPROVED: Bandpass filter for footstep frequencies (100-600Hz)
    
    // High-pass at ~100Hz to remove low rumble
    static float hp_prev = 0.0f;
    float highpass = sample - 0.98f * hp_prev;
    hp_prev = sample;
    
    // Low-pass at ~600Hz to focus on footstep range
    static float lp_prev = 0.0f;
    float lowpass = 0.6f * highpass + 0.4f * lp_prev;
    lp_prev = lowpass;
    
    // Update frequency buffer with bandpass-filtered content
    frequencyBuffer[frequencyBufferPos] = std::abs(lowpass);
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
    // WIDER energy range to catch more footsteps
    float energyScore = 0.0f;
    if (energy >= 0.01f && energy <= 0.2f) // MUCH WIDER: was 0.025-0.100
    {
        // More permissive scoring - linear instead of peak-focused
        if (energy <= 0.05f) {
            energyScore = energy / 0.05f; // Linear rise to 1.0 at 0.05
        } else {
            energyScore = 1.0f - ((energy - 0.05f) / 0.15f) * 0.3f; // Gentle falloff
        }
        energyScore = std::max(0.0f, energyScore);
    }
    
    // MORE GENEROUS frequency scoring
    float frequencyScore = std::min(1.0f, frequency * 15.0f); // was * 10.0f
    
    // LOWER weighting requirements - easier to get high confidence
    float confidence = (energyScore * 0.6f) + (frequencyScore * 0.4f);
    
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
    return backgroundNoise;
}

bool FootstepClassifier::isInCooldown() const 
{ 
    return cooldownCounter > 0; 
}