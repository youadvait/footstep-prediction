#include "FootstepClassifier.h"
#include <algorithm>
#include <numeric>
#include <iostream>

FootstepClassifier::FootstepClassifier()
{
    // OPTIMIZED: 40-500Hz frequency bands for maximum footstep detection
    energyBands[0].resize(48, 0.0f); // 40-120Hz
    energyBands[1].resize(48, 0.0f); // 120-250Hz 
    energyBands[2].resize(40, 0.0f); // 250-400Hz
    energyBands[3].resize(32, 0.0f); // 400-500Hz
    
    // Enhanced analysis buffers
    spectralBuffer.resize(64, 0.0f);
    onsetBuffer.resize(24, 0.0f); // Larger for better onset detection
    temporalBuffer.resize(128, 0.0f);
    noiseEstimationBuffer.resize(256, 0.0f);
    
    // Initialize filter states
    for (auto& bandStates : filterStates) {
        std::fill(bandStates.begin(), bandStates.end(), 0.0f);
    }
}

FootstepClassifier::~FootstepClassifier()
{
    // Safe cleanup
}

void FootstepClassifier::prepare(double sampleRate, int samplesPerBlock)
{
    // Validate parameters
    if (sampleRate > 0.0 && sampleRate <= 192000.0) {
        currentSampleRate = sampleRate;
    } else {
        currentSampleRate = 44100.0;
    }
    
    if (samplesPerBlock > 0 && samplesPerBlock <= 8192) {
        currentBlockSize = samplesPerBlock;
    } else {
        currentBlockSize = 512;
    }
    
    // Reset all buffers
    for (auto& band : energyBands) {
        std::fill(band.begin(), band.end(), 0.0f);
    }
    std::fill(energyBandPos.begin(), energyBandPos.end(), 0);
    std::fill(bandEnergies.begin(), bandEnergies.end(), 0.0f);
    
    std::fill(spectralBuffer.begin(), spectralBuffer.end(), 0.0f);
    spectralBufferPos = 0;
    
    std::fill(onsetBuffer.begin(), onsetBuffer.end(), 0.0f);
    onsetBufferPos = 0;
    
    std::fill(temporalBuffer.begin(), temporalBuffer.end(), 0.0f);
    temporalBufferPos = 0;
    
    std::fill(noiseEstimationBuffer.begin(), noiseEstimationBuffer.end(), 0.0f);
    noiseBufferPos = 0;
    
    // Reset filter states
    for (auto& bandStates : filterStates) {
        std::fill(bandStates.begin(), bandStates.end(), 0.0f);
    }
    
    // Reset detection state with optimized thresholds for 40-500Hz
    lastConfidence = 0.0f;
    lastEnergy = 0.0f;
    lastSpectralCentroid = 0.0f;
    lastSpectralRolloff = 0.0f;
    lastOnsetStrength = 0.0f;
    previousEnergy = 0.0f;
    backgroundNoiseLevel = 0.008f; // Based on your test data
    adaptiveThreshold = 0.2f; // Lower threshold for better detection
    cooldownCounter = 0;
    stepDurationCounter = 0;
}

bool FootstepClassifier::detectFootstep(float inputSample, float sensitivity)
{
    if (!isValidSample(inputSample)) {
        return false;
    }
    
    // SIMPLE TEST: Detect any signal above threshold
    float energy = std::abs(inputSample);
    lastEnergy = energy;
    
    // Simple threshold based on sensitivity
    float threshold = 0.01f * (1.0f - sensitivity); // More sensitive = lower threshold
    
    // Update confidence
    lastConfidence = energy / (threshold + 0.001f);
    
    // Cooldown check
    if (cooldownCounter > 0) {
        cooldownCounter--;
        return false;
    }
    
    bool isFootstep = energy > threshold;
    
    if (isFootstep) {
        cooldownCounter = static_cast<int>(currentSampleRate * 0.1f); // 100ms cooldown
        std::cout << "🎯 SIMPLE DETECTION | Energy=" << energy 
                  << " | Threshold=" << threshold 
                  << " | Sample=" << inputSample << std::endl;
    }
    
    return isFootstep;
}

void FootstepClassifier::updateMultiBandEnergy(float sample)
{
    // OPTIMIZED: Apply 40-500Hz specific filters
    for (int band = 0; band < 4; ++band) {
        float filteredSample = applyBandFilter(sample, band);
        float energy = filteredSample * filteredSample;
        
        // Update circular buffer
        energyBands[band][energyBandPos[band]] = energy;
        energyBandPos[band] = (energyBandPos[band] + 1) % energyBands[band].size();
        
        // Calculate RMS for this band
        float sum = 0.0f;
        for (float val : energyBands[band]) {
            sum += val;
        }
        bandEnergies[band] = std::sqrt(sum / energyBands[band].size());
    }
    
    lastEnergy = std::sqrt(sample * sample);
}

float FootstepClassifier::applyBandFilter(float sample, int bandIndex)
{
    // OPTIMIZED: Aggressive filters for 40-500Hz footstep range
    float* state = filterStates[bandIndex].data(); // [x1, x2, y1]
    float output = 0.0f;
    
    switch (bandIndex) {
        case 0: // 40-120Hz (Low impact/rumble) - Enhanced low-end
            output = 0.3f * sample + 0.2f * state[0] - 0.1f * state[1] + 0.6f * state[2];
            break;
        case 1: // 120-250Hz (Primary range) - Maximum sensitivity
            output = 0.5f * sample + 0.3f * state[0] - 0.15f * state[1] + 0.35f * state[2];
            break;
        case 2: // 250-400Hz (Detail/harmonics) - Good sensitivity
            output = 0.4f * sample + 0.25f * state[0] - 0.1f * state[1] + 0.45f * state[2];
            break;
        case 3: // 400-500Hz (Surface texture) - Moderate sensitivity
            output = 0.25f * sample + 0.15f * state[0] - 0.05f * state[1] + 0.65f * state[2];
            break;
    }
    
    // Update filter state
    state[1] = state[0];
    state[0] = sample;
    state[2] = output;
    
    return output;
}

float FootstepClassifier::calculateSpectralFeatures(float sample)
{
    // Update spectral buffer
    spectralBuffer[spectralBufferPos] = std::abs(sample);
    spectralBufferPos = (spectralBufferPos + 1) % spectralBuffer.size();
    
    // OPTIMIZED: Spectral centroid for 40-500Hz range
    float weightedSum = 0.0f;
    float magnitudeSum = 0.0f;
    
    for (size_t i = 0; i < spectralBuffer.size(); ++i) {
        float magnitude = spectralBuffer[i];
        float frequency = (i * currentSampleRate) / (2.0f * spectralBuffer.size());
        
        // Triple weight for 40-500Hz footstep range
        float freqWeight = 1.0f;
        if (frequency >= 40.0f && frequency <= 500.0f) {
            freqWeight = 3.0f; // Triple weight for our target range
        }
        
        weightedSum += frequency * magnitude * freqWeight;
        magnitudeSum += magnitude * freqWeight;
    }
    
    if (magnitudeSum > 0.001f) {
        lastSpectralCentroid = weightedSum / magnitudeSum;
        
        // Normalize for 40-500Hz range
        float normalizedCentroid = clamp((lastSpectralCentroid - 40.0f) / 460.0f, 0.0f, 1.0f);
        
        // Peak scoring around 150-300Hz (center of our range)
        float optimalRange = 0.45f; // (200-40)/460 ≈ 0.35-0.55
        return 1.0f - std::abs(optimalRange - normalizedCentroid) * 1.2f;
    }
    
    return 0.0f;
}

float FootstepClassifier::detectOnset(float sample)
{
    float currentEnergy = sample * sample;
    
    // Enhanced onset detection with exponential weighting
    float energyDiff = currentEnergy - previousEnergy;
    float smoothedDiff = std::max(0.0f, energyDiff);
    
    onsetBuffer[onsetBufferPos] = smoothedDiff;
    onsetBufferPos = (onsetBufferPos + 1) % onsetBuffer.size();
    
    // Weighted onset calculation (recent samples more important)
    float onsetSum = 0.0f;
    float weightSum = 0.0f;
    for (size_t i = 0; i < onsetBuffer.size(); ++i) {
        float weight = 1.0f - (i * 0.7f / onsetBuffer.size());
        onsetSum += onsetBuffer[i] * weight;
        weightSum += weight;
    }
    
    lastOnsetStrength = (weightSum > 0.0f) ? (onsetSum / weightSum) : 0.0f;
    previousEnergy = currentEnergy;
    
    // OPTIMIZED: More aggressive onset scaling for 40-500Hz
    return clamp(lastOnsetStrength * 200.0f, 0.0f, 1.0f);
}

float FootstepClassifier::calculateTemporalPattern()
{
    temporalBuffer[temporalBufferPos] = lastConfidence;
    temporalBufferPos = (temporalBufferPos + 1) % temporalBuffer.size();
    
    stepDurationCounter++;
    
    // OPTIMIZED: Temporal patterns for faster footstep detection
    float samplesIn25ms = currentSampleRate * 0.025f;  // Minimum duration
    float samplesIn120ms = currentSampleRate * 0.12f;  // Maximum single footstep
    
    if (stepDurationCounter >= samplesIn25ms && stepDurationCounter <= samplesIn120ms) {
        return 1.0f;
    } else if (stepDurationCounter > samplesIn120ms) {
        stepDurationCounter = 0;
        return 0.4f;
    }
    
    return 0.1f;
}

void FootstepClassifier::updateBackgroundNoise(float sample)
{
    float energy = sample * sample;
    
    noiseEstimationBuffer[noiseBufferPos] = energy;
    noiseBufferPos = (noiseBufferPos + 1) % noiseEstimationBuffer.size();
    
    // More conservative noise estimation
    std::vector<float> sortedNoise = noiseEstimationBuffer;
    std::sort(sortedNoise.begin(), sortedNoise.end());
    
    // Use 15th percentile for more aggressive detection
    size_t percentile15 = sortedNoise.size() * 15 / 100;
    backgroundNoiseLevel = std::sqrt(sortedNoise[percentile15]);
    
    // Faster adaptation
    float targetThreshold = 0.2f + (backgroundNoiseLevel * 1.2f);
    adaptiveThreshold += (targetThreshold - adaptiveThreshold) * 0.02f;
    adaptiveThreshold = clamp(adaptiveThreshold, 0.1f, 0.6f);
}

float FootstepClassifier::calculateAdvancedConfidence()
{
    // OPTIMIZED: Confidence calculation for 40-500Hz footstep likelihood
    float footstepLikelihood = calculateFootstepLikelihood(bandEnergies);
    
    // ENHANCED: Aggressive weighting for maximum detection
    float energyScore = footstepLikelihood * 0.7f;        // 70% - increased weight
    float spectralScore = (lastSpectralCentroid > 0) ? 0.12f : 0.0f; // 12% - spectral
    float onsetScore = lastOnsetStrength * 0.12f;         // 12% - onset
    float temporalScore = calculateTemporalPattern() * 0.06f; // 6% - temporal
    
    float confidence = energyScore + spectralScore + onsetScore + temporalScore;
    
    // OPTIMIZED: Noise adjustment for 40-500Hz detection
    float noiseRatio = backgroundNoiseLevel / 0.008f;
    float noiseAdjustment = clamp(1.5f - (noiseRatio * 0.3f), 0.9f, 2.0f);
    confidence *= noiseAdjustment;
    
    return clamp(confidence, 0.0f, 1.0f);
}

float FootstepClassifier::calculateFootstepLikelihood(const std::array<float, 4>& bands)
{
    // OPTIMIZED: 40-500Hz footstep signature analysis
    float lowImpact = bands[0];    // 40-120Hz (impact/rumble)
    float primaryEnergy = bands[1]; // 120-250Hz (primary footstep)
    float footstepDetail = bands[2]; // 250-400Hz (detail/harmonics)
    float surfaceTexture = bands[3]; // 400-500Hz (texture)
    
    // 40-500Hz footstep characteristics:
    
    // 1. Primary energy (120-250Hz) should be dominant
    float totalEnergy = lowImpact + primaryEnergy + footstepDetail + surfaceTexture + 0.001f;
    float primaryRatio = primaryEnergy / totalEnergy;
    
    float primaryDominance = 0.0f;
    if (primaryRatio > 0.2f) { // Primary should be at least 20%
        primaryDominance = clamp((primaryRatio - 0.2f) / 0.5f, 0.0f, 1.0f);
    }
    
    // 2. Good energy distribution across 40-500Hz spectrum
    float lowToPrimaryRatio = (primaryEnergy > 0.001f) ? clamp(lowImpact / primaryEnergy, 0.0f, 1.5f) : 0.0f;
    float detailToPrimaryRatio = (primaryEnergy > 0.001f) ? clamp(footstepDetail / primaryEnergy, 0.0f, 1.2f) : 0.0f;
    
    // Optimal ratios for 40-500Hz footsteps
    float lowScore = 1.0f - std::abs(0.8f - lowToPrimaryRatio);    // Target 80% of primary
    float detailScore = 1.0f - std::abs(0.75f - detailToPrimaryRatio); // Target 75% of primary
    
    // 3. Surface texture should be present but moderate
    float textureScore = clamp(surfaceTexture / (primaryEnergy + 0.001f), 0.0f, 0.6f) * 1.67f;
    
    // 4. OPTIMIZED: Energy gate for 40-500Hz range (lower threshold)
    float rmsThreshold = 0.025f; // Lower threshold for better detection
    float energyGate = (std::sqrt(totalEnergy) > rmsThreshold) ? 1.0f : 0.0f;
    
    // ENHANCED: Optimized weights for 40-500Hz footstep detection
    float likelihood = (primaryDominance * 0.4f) + (lowScore * 0.25f) + (detailScore * 0.2f) + 
                      (textureScore * 0.1f) + (energyGate * 0.05f);
    
    return clamp(likelihood, 0.0f, 1.0f);
}

// Utility methods
float FootstepClassifier::clamp(float value, float min, float max) const
{
    return std::max(min, std::min(value, max));
}

bool FootstepClassifier::isValidSample(float sample) const
{
    return !std::isnan(sample) && !std::isinf(sample) && std::abs(sample) < 10.0f;
}