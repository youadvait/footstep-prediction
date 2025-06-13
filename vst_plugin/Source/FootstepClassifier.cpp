#include "FootstepClassifier.h"
#include <algorithm>
#include <iostream>

FootstepClassifier::FootstepClassifier()
{
    // Initialize buffers for analysis
    energyBuffer.resize(256, 0.0f);
    frequencyBuffer.resize(128, 0.0f);
    transientBuffer.resize(64, 0.0f);
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
    std::fill(transientBuffer.begin(), transientBuffer.end(), 0.0f);  // NEW
    energyBufferPos = 0;
    frequencyBufferPos = 0;
    transientBufferPos = 0;  // NEW
    currentEnergy = 0.0f;
    lastConfidence = 0.0f;
    lastEnergy = 0.0f;
    cooldownCounter = 0;
    
    // NEW: Initialize high-frequency filter for gunshot detection
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    
    highFreqFilter.prepare(spec);
    highFreqFilter.reset();
    highFreqFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, 1000.0f, 0.7f  // High-pass at 1kHz for gunshot detection
    );
}

bool FootstepClassifier::detectFootstep(float inputSample, float sensitivity)
{
    if (currentSampleRate <= 0) return false;
    
    // Cooldown check
    if (cooldownCounter > 0)
    {
        cooldownCounter--;
        return false;
    }
    
    // ENHANCED: Multi-stage analysis with gunshot rejection
    float energy = calculateEnergy(inputSample);
    float lowFreqContent = calculateFrequencyContent(inputSample);  // Rename existing method
    float highFreqContent = calculateGunshotFrequencyContent(inputSample);  // NEW
    float transientDuration = analyzeTransientDuration(energy);  // NEW
    
    // NEW: Gunshot rejection logic
    bool isLikelyGunshot = detectGunshot(energy, highFreqContent, transientDuration);
    
    if (isLikelyGunshot)
    {
        // GUNSHOT DETECTED: Set longer cooldown and reject
        cooldownCounter = static_cast<int>(currentSampleRate * 0.5); // 500ms cooldown for gunshots
        std::cout << "🚫 GUNSHOT REJECTED | Energy: " << energy 
                  << " | HighFreq: " << highFreqContent 
                  << " | Duration: " << transientDuration << std::endl;
        return false;
    }
    
    // FOOTSTEP DETECTION: Only if not a gunshot
    float footstepConfidence = calculateFootstepConfidence(energy, lowFreqContent, transientDuration);
    
    // MORE SELECTIVE threshold
    float threshold = 0.75f + (1.0f - sensitivity) * 0.2f; // Range: 0.55 to 0.75
    
    bool isFootstep = footstepConfidence > threshold;
    
    if (isFootstep)
    {
        cooldownCounter = static_cast<int>(currentSampleRate * 0.15); // 150ms cooldown
        std::cout << "👟 FOOTSTEP DETECTED | Confidence: " << footstepConfidence 
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




float FootstepClassifier::calculateGunshotFrequencyContent(float sample)
{
    // NEW: Detect high-frequency content typical of gunshots
    float highFreqSample = highFreqFilter.processSample(sample);
    return std::abs(highFreqSample);
}

float FootstepClassifier::analyzeTransientDuration(float energy)
{
    // NEW: Analyze how quickly energy changes
    transientBuffer[transientBufferPos] = energy;
    transientBufferPos = (transientBufferPos + 1) % transientBuffer.size();
    
    // Calculate energy variance (high variance = sharp transients like gunshots)
    float mean = 0.0f;
    for (float e : transientBuffer) {
        mean += e;
    }
    mean /= transientBuffer.size();
    
    float variance = 0.0f;
    for (float e : transientBuffer) {
        variance += (e - mean) * (e - mean);
    }
    variance /= transientBuffer.size();
    
    return variance * 1000.0f; // Scale for easier interpretation
}

bool FootstepClassifier::detectGunshot(float energy, float highFreqContent, float transientDuration)
{
    // GUNSHOT CHARACTERISTICS:
    bool highEnergy = energy > 0.15f;           // Much higher than footsteps
    bool highFrequency = highFreqContent > 0.1f; // Significant high-freq content
    bool sharpTransient = transientDuration > 5.0f; // High variance = sharp burst
    
    // ALL conditions must be met for gunshot
    return highEnergy && highFrequency && sharpTransient;
}

float FootstepClassifier::calculateFootstepConfidence(float energy, float lowFreqContent, float transientDuration)
{
    // FOOTSTEP-SPECIFIC confidence calculation
    float energyScore = 0.0f;
    if (energy >= 0.025f && energy <= 0.100f) {
        energyScore = 1.0f - std::abs(0.0625f - energy) / 0.0375f;
    }
    
    float freqScore = std::min(1.0f, lowFreqContent * 10.0f);
    
    float durationScore = 0.0f;
    if (transientDuration < 3.0f) { // Footsteps have low variance (smooth)
        durationScore = 1.0f - (transientDuration / 3.0f);
    }
    
    // Weighted combination favoring energy and frequency
    return (energyScore * 0.5f) + (freqScore * 0.3f) + (durationScore * 0.2f);
}
