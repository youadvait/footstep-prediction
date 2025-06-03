#pragma once

#include <vector>
#include <cmath>
#include <array>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class FootstepClassifier
{
public:
    FootstepClassifier();
    ~FootstepClassifier();
    
    void prepare(double sampleRate, int samplesPerBlock);
    bool detectFootstep(float inputSample, float sensitivity);
    
    // ADDED: Missing getter methods for debug output
    float getLastConfidence() const { return lastConfidence; }
    float getLastEnergy() const { return lastEnergy; }
    float getLastSpectralCentroid() const { return lastSpectralCentroid; }
    float getBackgroundNoise() const { return backgroundNoiseLevel; }
    bool isInCooldown() const { return cooldownCounter > 0; }
    std::array<float, 4> getBandEnergies() const { return bandEnergies; }
    
private:
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    // Multi-band energy analysis (COD-specific frequency bands)
    std::array<std::vector<float>, 4> energyBands; // 4 frequency bands
    std::array<size_t, 4> energyBandPos = {0, 0, 0, 0};
    std::array<float, 4> bandEnergies = {0.0f, 0.0f, 0.0f, 0.0f};
    
    // Spectral analysis
    std::vector<float> spectralBuffer;
    size_t spectralBufferPos = 0;
    float lastSpectralCentroid = 0.0f;
    float lastSpectralRolloff = 0.0f;
    
    // Onset detection
    std::vector<float> onsetBuffer;
    size_t onsetBufferPos = 0;
    float lastOnsetStrength = 0.0f;
    float previousEnergy = 0.0f;
    
    // Temporal pattern analysis
    std::vector<float> temporalBuffer;
    size_t temporalBufferPos = 0;
    int stepDurationCounter = 0;
    
    // Adaptive background noise estimation
    float backgroundNoiseLevel = 0.0f;
    float adaptiveThreshold = 0.3f;
    std::vector<float> noiseEstimationBuffer;
    size_t noiseBufferPos = 0;
    
    // Detection state - THESE NEED TO EXIST FOR GETTERS
    float lastConfidence = 0.0f;
    float lastEnergy = 0.0f;
    int cooldownCounter = 0;
    
    // Filter states for multi-band analysis (simple IIR filters)
    std::array<std::array<float, 3>, 4> filterStates; // [band][state: x1, x2, y1]
    
    // Enhanced helper methods
    void updateMultiBandEnergy(float sample);
    float calculateSpectralFeatures(float sample);
    float detectOnset(float sample);
    float calculateTemporalPattern();
    void updateBackgroundNoise(float sample);
    float calculateAdvancedConfidence();
    
    // COD-specific frequency filters
    float applyBandFilter(float sample, int bandIndex);
    float calculateFootstepLikelihood(const std::array<float, 4>& bandEnergies);
    
    // Utility methods
    float clamp(float value, float min, float max) const;
    bool isValidSample(float sample) const;
};