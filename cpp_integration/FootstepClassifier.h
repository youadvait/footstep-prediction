
#pragma once
#include "FootstepFeatureExtractor.h"
#include <array>

class FootstepClassifier {
public:
    FootstepClassifier();
    ~FootstepClassifier();
    
    // Initialize with model parameters
    bool loadModel(const std::string& modelPath);
    
    // Classify audio segment - returns confidence (0.0 to 1.0)
    float classifyFootstep(const std::array<float, FootstepFeatureExtractor::FEATURE_SIZE>& features);
    
    // Real-time processing
    float processAudioFrame(const float* audioData, int numSamples);
    
private:
    bool modelLoaded;
    
    // Feature scaling parameters
    std::array<float, FootstepFeatureExtractor::FEATURE_SIZE> featureMeans;
    std::array<float, FootstepFeatureExtractor::FEATURE_SIZE> featureStds;
    
    // Simplified decision tree structure for real-time processing
    struct DecisionNode {
        int featureIndex;
        float threshold;
        float leftValue;   // If feature <= threshold
        float rightValue;  // If feature > threshold
        bool isLeaf;
    };
    
    std::vector<DecisionNode> decisionTrees;
    void initializeSimplifiedModel();
};
