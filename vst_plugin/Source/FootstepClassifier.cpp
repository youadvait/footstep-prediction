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
    
    // ULTRA-SELECTIVE: Much higher thresholds
    float baseThreshold = 0.85f; // Very high base threshold
    float threshold = baseThreshold + (1.0f - sensitivity) * 0.1f; // Range: 0.85 to 0.95
    
    if (cooldownCounter > 0)
    {
        cooldownCounter--;
        return false;
    }
    
    // MUCH MORE RESTRICTIVE confidence calculation
    float confidence = calculateUltraSelectiveConfidence(energy, frequency);
    
    bool isFootstep = confidence > threshold;
    
    if (isFootstep)
    {
        cooldownCounter = static_cast<int>(currentSampleRate * 0.2); // 200ms cooldown
        std::cout << "👟 ULTRA-SELECTIVE FOOTSTEP | Confidence: " << confidence 
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
    // PROVEN footstep energy range from working version
    float energyScore = 0.0f;
    if (energy >= 0.02f && energy <= 0.15f) // Moderate range - not too narrow, not too wide
    {
        // Optimal range for most footsteps
        if (energy >= 0.04f && energy <= 0.08f) {
            energyScore = 1.0f; // Perfect footstep energy
        } else if (energy < 0.04f) {
            energyScore = (energy - 0.02f) / 0.02f; // Ramp up from 0.02
        } else {
            energyScore = 1.0f - ((energy - 0.08f) / 0.07f) * 0.5f; // Gentle falloff
        }
        energyScore = std::max(0.0f, energyScore);
    }
    
    // Frequency content for footstep discrimination
    float frequencyScore = std::min(1.0f, frequency * 12.0f);
    
    // Balanced combination
    float confidence = (energyScore * 0.65f) + (frequencyScore * 0.35f);
    
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

float FootstepClassifier::calculateUltraSelectiveConfidence(float energy, float frequency)
{
    // ULTRA-RESTRICTIVE: Only very specific footstep characteristics
    
    // Very narrow energy range for footsteps only
    float energyScore = 0.0f;
    if (energy >= 0.035f && energy <= 0.065f) { // VERY NARROW range
        // Must be in perfect footstep energy range
        energyScore = 1.0f - std::abs(0.05f - energy) / 0.015f;
        energyScore = std::max(0.0f, energyScore);
    }
    
    // Strict frequency requirements
    float freqScore = 0.0f;
    if (frequency >= 0.03f && frequency <= 0.08f) { // VERY NARROW range
        freqScore = std::min(1.0f, frequency * 20.0f); // Much higher multiplier
    }
    
    // BOTH must be excellent for high confidence
    float confidence = energyScore * freqScore; // Multiplicative - both required
    
    return confidence;
}