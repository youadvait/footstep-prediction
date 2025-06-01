
#pragma once
#include <vector>
#include <array>
#include <cmath>

class FootstepFeatureExtractor {
public:
    static constexpr int N_MFCC = 13;
    static constexpr int FEATURE_SIZE = 78;
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr float WINDOW_DURATION = 0.4f;
    
    FootstepFeatureExtractor();
    ~FootstepFeatureExtractor();
    
    // Extract MFCC features from audio segment
    std::array<float, FEATURE_SIZE> extractFeatures(const float* audioData, int numSamples);
    
private:
    void computeMFCC(const float* audioData, int numSamples, float* mfccOutput);
    void computeStatistics(const float* mfccMatrix, int numFrames, float* features);
    
    // FFT and mel filter bank data
    std::vector<float> melFilterBank;
    std::vector<float> dctMatrix;
    void initializeMelFilters();
    void initializeDCT();
};
