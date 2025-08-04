#pragma once

#include <vector>
#include <memory>
#include <string>
#include <fstream>

// Simplified ML classifier without TensorFlow Lite dependencies
class MLFootstepClassifier
{
public:
    MLFootstepClassifier();
    ~MLFootstepClassifier();
    
    // Initialize ML model
    bool loadModel(const std::string& modelPath);
    void prepare(double sampleRate, int samplesPerBlock);
    
    // Main detection method (replaces FootstepClassifier)
    bool detectFootstep(float inputSample, float sensitivity);
    
    // Compatibility methods
    float getLastConfidence() const { return lastConfidence; }
    float getLastEnergy() const { return lastEnergy; }
    float getBackgroundNoise() const { return 0.015f; }
    bool isInCooldown() const { return cooldownCounter > 0; }
    
    // Debug methods
    void printDebugStats() const;
    void resetDebugStats();
    void enableTestMode(bool enable) { testMode = enable; }
    
private:
    // Audio processing parameters
    static constexpr int BUFFER_SIZE = 2048;  // Smaller buffer for real-time
    static constexpr int FEATURE_SIZE = 32;   // Simplified features
    
    // Audio buffer for feature extraction
    std::vector<float> audioBuffer;
    size_t bufferPos = 0;
    
    // Detection state
    float lastConfidence = 0.0f;
    float lastEnergy = 0.0f;
    int cooldownCounter = 0;
    int processingCounter = 0;  // Move from static to instance variable
    double currentSampleRate = 44100.0;
    
    // ML model weights (simplified - pre-computed from your trained model)
    std::vector<float> modelWeights;
    std::vector<float> modelBias;
    bool modelLoaded = false;
    
    // Debug counters
    int totalDetections = 0;
    int falsePositiveCounter = 0;
    bool testMode = false;
    
    // Feature extraction
    void extractFeatures(const float* audio, int length, float* features);
    float runSimpleInference(const float* features);
    
    // Utility methods
    float calculateRMS(const float* audio, int length);
    float calculateSpectralCentroid(const float* audio, int length);
    float calculateZeroCrossingRate(const float* audio, int length);
};