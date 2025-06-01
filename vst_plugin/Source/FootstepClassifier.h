#pragma once

#include <vector>
#include <array>
#include <string>

class FootstepClassifier
{
public:
    static constexpr int N_FEATURES = 78; // Keep for compatibility
    
    FootstepClassifier();
    ~FootstepClassifier();
    
    bool loadModel(const std::string& modelPath);
    
    // MAIN DETECTION METHOD - uses raw audio buffer
    float classifyFootstep(const float* audioBuffer, int bufferSize, float sampleRate);
    
    bool isModelLoaded() const { return modelLoaded; }
    float getLastConfidence() const { return lastConfidence; }
    
    void validateModel() const;
    void printModelInfo() const;
    
    // Legacy method for compatibility
    float classifyFootstep(const std::array<float, N_FEATURES>& features);
    
private:
    bool modelLoaded = false;
    float lastConfidence = 0.0f;
    
    // Detection state
    float lastRMS = 0.0f;
    float lastFootstepEnergy = 0.0f;
    std::vector<float> energyHistory;
    std::vector<float> freqHistory;
    int silenceCounter = 0;
    
    // COD footstep characteristics (from your analysis)
    static constexpr float COD_RMS_MIN = 0.025f;        // Your range: 0.025-0.083
    static constexpr float COD_RMS_MAX = 0.100f;
    static constexpr float COD_FOOTSTEP_FREQ_LOW = 100.0f;   // Footstep frequency range
    static constexpr float COD_FOOTSTEP_FREQ_HIGH = 1000.0f;
    static constexpr float COD_ONSET_THRESHOLD = 3.0f;      // Sudden energy change
    
    // Detection methods (REMOVED isInFootstepFrequencyRange - not used!)
    float analyzeEnergyAndFrequency(const float* audioBuffer, int bufferSize, float sampleRate);
    bool detectOnset(float currentRMS);
    float analyzeFootstepFrequencies(const float* audioBuffer, int bufferSize, float sampleRate);
    float calculateBandEnergy(const float* audioBuffer, int bufferSize, float sampleRate, 
                             float lowFreq, float highFreq);
};
