#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>
#include <cmath>

class MFCCExtractor
{
public:
    static constexpr int N_MFCC = 13;
    static constexpr int N_FEATURES = 78;  // 13 * 6 (mean, std, max, min, delta_mean, delta2_mean)
    static constexpr int WINDOW_SIZE = 2048;
    static constexpr int HOP_SIZE = 512;
    static constexpr int N_MEL_FILTERS = 40;
    
    MFCCExtractor();
    ~MFCCExtractor();
    
    void prepare(double sampleRate);
    std::array<float, N_FEATURES> extractFeatures(const float* audioData, int numSamples);
    
private:
    double sampleRate = 44100.0;
    
    // FFT processing
    juce::dsp::FFT fft;
    std::vector<float> fftBuffer;
    std::vector<float> window;
    std::vector<float> magnitudeSpectrum;
    
    // Mel filter bank
    std::vector<std::vector<float>> melFilterBank;
    std::vector<float> melEnergies;
    
    // DCT matrix for MFCC computation
    std::vector<std::vector<float>> dctMatrix;
    
    // Feature computation buffers
    std::vector<std::vector<float>> mfccFrames;
    std::vector<float> currentMFCC;
    std::vector<float> prevMFCC;
    std::vector<float> prevPrevMFCC;
    
    // Helper methods
    void initializeMelFilterBank();
    void initializeDCT();
    void processSingleFrame(const float* frameData);  // NEW METHOD
    void computeFeatureStatistics(std::array<float, N_FEATURES>& features);
    float melScale(float frequency);
    float invMelScale(float mel);
};