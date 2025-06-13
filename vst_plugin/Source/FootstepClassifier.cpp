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
    
    // Cooldown check - MUCH SHORTER
    if (cooldownCounter > 0)
    {
        cooldownCounter--;
        return false;
    }
    
    // SIMPLIFIED: Basic analysis only
    float energy = calculateEnergy(inputSample);
    float frequency = calculateFrequencyContent(inputSample);
    
    // BASIC gunshot rejection - only for extreme cases
    float highFreqContent = calculateGunshotFrequencyContent(inputSample);
    bool isObviousGunshot = (energy > 0.3f && highFreqContent > 0.5f); // Very high thresholds
    
    if (isObviousGunshot)
    {
        // SHORTER gunshot cooldown
        cooldownCounter = static_cast<int>(currentSampleRate * 0.2); // 200ms (was 500ms)
        std::cout << "🚫 OBVIOUS GUNSHOT REJECTED | Energy: " << energy << std::endl;
        return false;
    }
    
    // MUCH MORE PERMISSIVE footstep detection
    float confidence = calculateFootstepConfidence(energy, frequency, 0.0f); // Ignore duration
    
    // MUCH LOWER threshold - more sensitive
    float threshold = 0.3f + (1.0f - sensitivity) * 0.4f; // Range: 0.3 to 0.7 (was 0.55-0.75)
    
    bool isFootstep = confidence > threshold;
    
    if (isFootstep)
    {
        // MUCH SHORTER footstep cooldown
        cooldownCounter = static_cast<int>(currentSampleRate * 0.05); // 50ms (was 150ms)
        std::cout << "👟 FOOTSTEP DETECTED | Confidence: " << confidence 
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
    // MUCH LESS AGGRESSIVE: Only reject extremely obvious gunshots
    
    bool extremeEnergy = energy > 0.5f;        // Very high threshold (was 0.15f)
    bool extremeHighFreq = highFreqContent > 0.8f; // Very high threshold (was 0.1f)
    
    // ONLY reject if BOTH conditions are extreme
    return extremeEnergy && extremeHighFreq;
}

float FootstepClassifier::calculateFootstepConfidence(float energy, float lowFreqContent, float transientDuration)
{
    // SIMPLIFIED: Focus on energy and frequency only
    
    // MUCH MORE PERMISSIVE energy range
    float energyScore = 0.0f;
    if (energy >= 0.01f && energy <= 0.4f) { // Wider range (was 0.025-0.100)
        // Give high score for any reasonable energy level
        energyScore = std::min(1.0f, energy * 5.0f); // Linear scaling
    }
    
    // PERMISSIVE frequency score
    float freqScore = std::min(1.0f, lowFreqContent * 8.0f); // More sensitive
    
    // IGNORE duration analysis for now - it was too restrictive
    
    // SIMPLE combination - heavily favor energy
    return (energyScore * 0.8f) + (freqScore * 0.2f);
}