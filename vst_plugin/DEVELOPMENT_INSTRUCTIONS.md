
# VST2 FOOTSTEP DETECTOR - DEVELOPMENT INSTRUCTIONS

## Prerequisites
1. Download JUCE Framework from: https://juce.com/get-juce
2. Install Visual Studio 2022 (Community Edition is fine)
3. Install Windows SDK

## Setup Steps

### 1. JUCE Setup
1. Extract JUCE to C:/JUCE (or update paths in .jucer file)
2. Open Projucer.exe from JUCE installation
3. Open FootstepDetector.jucer from vst_plugin folder
4. Click "Save and Open in IDE" button

### 2. Implementation Order
1. Implement FootstepFeatureExtractor.cpp
   - MFCC calculation using JUCE DSP classes
   - Real-time optimized feature extraction
   
2. Implement FootstepClassifier.cpp  
   - Load simplified model parameters
   - Fast classification using decision tree ensemble
   
3. Implement PluginProcessor.cpp
   - Audio processing pipeline
   - Integration with EQ for footstep amplification
   - Parameter management
   
4. Implement PluginEditor.cpp
   - Simple UI with sensitivity and gain controls
   - Real-time detection visualization

### 3. Key Implementation Points

#### Real-Time MFCC Extraction
- Use sliding window approach
- Pre-allocate all buffers
- Use JUCE FFT classes for performance
- Target <2ms processing time per frame

#### Classification
- Convert Random Forest to simple decision rules
- Use lookup tables where possible  
- Implement confidence smoothing

#### EQ Integration
- Use JUCE IIR filters for footstep frequency bands
- Apply gain only when footstep detected
- Smooth gain changes to prevent artifacts

### 4. Performance Targets
- Total latency: <8ms
- CPU usage: <5% on modern CPU
- Detection accuracy: >85% (validated with your dataset)
- False positive rate: <10%

### 5. Testing Strategy
1. Unit test each component separately
2. Test with your WAV files as input
3. Real-time testing with game audio
4. Integration testing in EqualizerAPO

## Next Immediate Steps:
1. Download and setup JUCE
2. Start with FootstepFeatureExtractor implementation
3. Create simple test harness for validation
