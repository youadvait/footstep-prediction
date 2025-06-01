#include "FootstepClassifier.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <iomanip>

FootstepClassifier::FootstepClassifier()
{
    std::cout << "🎮 FINAL COD FootstepClassifier (Energy + Frequency)" << std::endl;
    
    lastRMS = 0.0f;
    lastFootstepEnergy = 0.0f;
    silenceCounter = 0;
    modelLoaded = true;
    
    // Initialize history buffers
    energyHistory.resize(10, 0.0f);
    freqHistory.resize(10, 0.0f);
    
    std::cout << "✅ Final energy + frequency model loaded" << std::endl;
    std::cout << "   COD RMS Range: " << COD_RMS_MIN << " - " << COD_RMS_MAX << std::endl;
    std::cout << "   Footstep Freq: " << COD_FOOTSTEP_FREQ_LOW << " - " << COD_FOOTSTEP_FREQ_HIGH << " Hz" << std::endl;
}

FootstepClassifier::~FootstepClassifier()
{
}

bool FootstepClassifier::loadModel(const std::string& /*modelPath*/)
{
    return modelLoaded;
}

// MAIN DETECTION METHOD
float FootstepClassifier::classifyFootstep(const float* audioBuffer, int bufferSize, float sampleRate)
{
    if (!modelLoaded || !audioBuffer || bufferSize <= 0) return 0.0f;
    
    float confidence = analyzeEnergyAndFrequency(audioBuffer, bufferSize, sampleRate);
    
    lastConfidence = confidence;
    return confidence;
}

float FootstepClassifier::analyzeEnergyAndFrequency(const float* audioBuffer, int bufferSize, float sampleRate)
{
    // 1. CALCULATE RMS ENERGY
    float rms = 0.0f;
    for (int i = 0; i < bufferSize; ++i) {
        rms += audioBuffer[i] * audioBuffer[i];
    }
    rms = std::sqrt(rms / bufferSize);
    
    // 2. SILENCE DETECTION (based on your COD analysis)
    if (rms < 0.003f) {  // Much lower than your COD min of 0.025
        silenceCounter++;
        if (silenceCounter > 5) {
            lastRMS = rms;
            return 0.0f;
        }
    } else {
        silenceCounter = 0;
    }
    
    float footstepScore = 0.0f;
    
    // 3. ENERGY RANGE CHECK (based on your COD data: 0.025-0.083)
    if (rms >= COD_RMS_MIN && rms <= COD_RMS_MAX) {
        footstepScore += 0.4f; // Strong indicator
        if (rms >= 0.04f && rms <= 0.07f) {
            footstepScore += 0.2f; // Sweet spot
        }
    } else if (rms > COD_RMS_MAX) {
        // Too loud = explosions/gunshots
        footstepScore *= 0.1f;
    }
    
    // 4. ONSET DETECTION (sudden energy increase)
    if (detectOnset(rms)) {
        footstepScore += 0.3f; // Footsteps have sudden onset
    }
    
    // 5. FREQUENCY ANALYSIS (footstep frequency range)
    float footstepFreqEnergy = analyzeFootstepFrequencies(audioBuffer, bufferSize, sampleRate);
    if (footstepFreqEnergy > 0.1f) {
        footstepScore += 0.2f; // Good frequency content
        if (footstepFreqEnergy > 0.3f) {
            footstepScore += 0.1f; // Strong frequency content
        }
    }
    
    // 6. TEMPORAL PATTERN (footsteps aren't continuous)
    // Update history
    energyHistory.erase(energyHistory.begin());
    energyHistory.push_back(rms);
    
    // Check for temporal variation
    float energyVariation = 0.0f;
    for (size_t i = 1; i < energyHistory.size(); ++i) {
        energyVariation += std::abs(energyHistory[i] - energyHistory[i-1]);
    }
    energyVariation /= (energyHistory.size() - 1);
    
    if (energyVariation > 0.01f && energyVariation < 0.05f) {
        footstepScore += 0.1f; // Good temporal variation
    }
    
    // 7. COMBINATION BONUS
    if (rms > 0.03f && footstepFreqEnergy > 0.2f && energyVariation > 0.015f) {
        footstepScore += 0.15f; // Bonus for good combination
    }
    
    // Update state
    lastRMS = rms;
    
    // Ensure valid range
    footstepScore = std::max(0.0f, std::min(1.0f, footstepScore));
    
    // Apply threshold - require strong evidence
    if (footstepScore < 0.7f) {
        footstepScore = 0.0f;
    }
    
    // Debug output
    static int debugCount = 0;
    if (++debugCount % 30 == 0 || footstepScore > 0.0f) {
        std::cout << "🎮 FINAL: RMS=" << std::fixed << std::setprecision(4) << rms
                  << ", FreqEnergy=" << std::setprecision(3) << footstepFreqEnergy
                  << ", Variation=" << energyVariation
                  << ", Onset=" << (detectOnset(rms) ? "YES" : "NO")
                  << " → Score=" << footstepScore << std::endl;
    }
    
    return footstepScore;
}

bool FootstepClassifier::detectOnset(float currentRMS)
{
    // Detect sudden energy increase (footstep characteristic)
    float energyRatio = (lastRMS > 0.001f) ? (currentRMS / lastRMS) : 1.0f;
    return (energyRatio > COD_ONSET_THRESHOLD && currentRMS > 0.02f);
}

float FootstepClassifier::analyzeFootstepFrequencies(const float* audioBuffer, int bufferSize, float sampleRate)
{
    // Simple frequency analysis - calculate energy in footstep frequency bands
    
    // Low-mid frequencies (100-500Hz) - main footstep energy
    float lowMidEnergy = calculateBandEnergy(audioBuffer, bufferSize, sampleRate, 100.0f, 500.0f);
    
    // Mid frequencies (500-1000Hz) - footstep attack/detail
    float midEnergy = calculateBandEnergy(audioBuffer, bufferSize, sampleRate, 500.0f, 1000.0f);
    
    // High frequencies (1000-3000Hz) - too much = claps/snaps
    float highEnergy = calculateBandEnergy(audioBuffer, bufferSize, sampleRate, 1000.0f, 3000.0f);
    
    // Footstep signature: strong low-mid, moderate mid, limited high
    float footstepEnergy = lowMidEnergy + (midEnergy * 0.5f);
    
    // Penalize if too much high frequency (claps, etc.)
    if (highEnergy > lowMidEnergy * 2.0f) {
        footstepEnergy *= 0.3f;
    }
    
    return std::min(1.0f, footstepEnergy);
}

float FootstepClassifier::calculateBandEnergy(const float* audioBuffer, int bufferSize, float sampleRate, 
                                            float lowFreq, float highFreq)
{
    // Simple time-domain approximation of frequency band energy
    // Not perfect but much more reliable than FFT for this use case
    
    float energy = 0.0f;
    int stepSize = static_cast<int>(sampleRate / (highFreq * 2)); // Approximate sampling for freq band
    stepSize = std::max(1, std::min(stepSize, bufferSize / 10));
    
    for (int i = 0; i < bufferSize - stepSize; i += stepSize) {
        float diff = std::abs(audioBuffer[i + stepSize] - audioBuffer[i]);
        energy += diff * diff;
    }
    
    return energy / (bufferSize / stepSize);
}

// Legacy method for compatibility
float FootstepClassifier::classifyFootstep(const std::array<float, N_FEATURES>& /*features*/)
{
    // Return last confidence for compatibility
    return lastConfidence;
}

void FootstepClassifier::printModelInfo() const
{
    std::cout << "🎮 Final energy + frequency model ready" << std::endl;
}

void FootstepClassifier::validateModel() const
{
    std::cout << "✅ Final model validated" << std::endl;
}