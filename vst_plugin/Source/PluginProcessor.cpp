#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <fstream>

// FIX: Ensure JUCE debug macros are available
#if JUCE_DEBUG
    #include <juce_core/juce_core.h>
#endif

FootstepDetectorAudioProcessor::FootstepDetectorAudioProcessor()
{
    std::cout << "🎮 FootstepDetector Enhanced Version - Initializing..." << std::endl;
    
    footstepClassifier = std::make_unique<FootstepClassifier>();
    
    // Initialize enhanced multi-band amplification filters (4 bands × 2 channels)
    amplificationFilters.resize(8);
    for (auto& filterState : amplificationFilters) {
        filterState.resize(3, 0.0f); // [x1, x2, y1] for each IIR filter
    }
    
    // Reset counters
    debugCounter = 0;
    detectionCount = 0;
    
    // FIXED: Multiple output methods for visibility
    std::cout << "✅ Enhanced FootstepDetector initialized with multi-band amplification!" << std::endl;
    std::cout.flush();
    std::cerr << "✅ Enhanced FootstepDetector initialized with multi-band amplification!" << std::endl;
    std::cerr.flush();
    
    // ADDED: File-based logging for debugging
    debugFile.open("/tmp/footstep_debug.log", std::ios::app);
    if (debugFile.is_open()) {
        debugFile << "=== FootstepDetector Session Started ===" << std::endl;
        debugFile.flush();
    }
}

FootstepDetectorAudioProcessor::~FootstepDetectorAudioProcessor()
{
    std::cout << "🔧 Enhanced FootstepDetector shutting down..." << std::endl;
    std::cerr << "🔧 Enhanced FootstepDetector shutting down..." << std::endl;
    
    if (debugFile.is_open()) {
        debugFile << "=== Session Ended | Total Detections: " << detectionCount << " ===" << std::endl;
        debugFile.close();
    }
}

const juce::String FootstepDetectorAudioProcessor::getName() const
{
#ifdef JucePlugin_Name
    return JucePlugin_Name;
#else
    return "FootstepDetector";
#endif
}

bool FootstepDetectorAudioProcessor::acceptsMidi() const
{
    return false;
}

bool FootstepDetectorAudioProcessor::producesMidi() const
{
    return false;
}

bool FootstepDetectorAudioProcessor::isMidiEffect() const
{
    return false;
}

double FootstepDetectorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FootstepDetectorAudioProcessor::getNumPrograms()
{
    return 1;
}

int FootstepDetectorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FootstepDetectorAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String FootstepDetectorAudioProcessor::getProgramName(int index)
{
    return {};
}

void FootstepDetectorAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void FootstepDetectorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    std::string prepareMsg = "🎵 Preparing enhanced audio processing: " + std::to_string(sampleRate) + "Hz, " + std::to_string(samplesPerBlock) + " samples";
    
    std::cout << prepareMsg << std::endl;
    std::cerr << prepareMsg << std::endl;
    
    if (debugFile.is_open()) {
        debugFile << prepareMsg << std::endl;
        debugFile.flush();
    }
    
    // Enhanced preparation
    if (footstepClassifier != nullptr)
    {
        footstepClassifier->prepare(sampleRate, samplesPerBlock);
        
        std::string classifierMsg = "✅ Enhanced FootstepClassifier prepared for professional detection";
        std::cout << classifierMsg << std::endl;
        std::cerr << classifierMsg << std::endl;
        
        if (debugFile.is_open()) {
            debugFile << classifierMsg << std::endl;
            debugFile.flush();
        }
    }
    
    // Reset enhanced amplification filter states
    for (auto& filterState : amplificationFilters) {
        std::fill(filterState.begin(), filterState.end(), 0.0f);
    }
    
    // Reset debug counters
    debugCounter = 0;
    detectionCount = 0;
    
    std::string completeMsg = "✅ Enhanced audio preparation complete";
    std::cout << completeMsg << std::endl;
    std::cerr << completeMsg << std::endl;
    
    if (debugFile.is_open()) {
        debugFile << completeMsg << std::endl;
        debugFile.flush();
    }
}

void FootstepDetectorAudioProcessor::releaseResources()
{
    std::string releaseMsg = "🔧 Releasing enhanced audio resources";
    std::cout << releaseMsg << std::endl;
    std::cerr << releaseMsg << std::endl;
    
    if (debugFile.is_open()) {
        debugFile << releaseMsg << std::endl;
        debugFile.flush();
    }
}

bool FootstepDetectorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void FootstepDetectorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = buffer.getNumChannels();
    float sensitivity = juce::jlimit(0.0f, 1.0f, sensitivityParam);
    float gain = juce::jlimit(1.0f, 8.0f, gainParam); // INCREASED for client delivery
    
    debugCounter += buffer.getNumSamples();
    
    // FIXED: MUCH more frequent monitoring (every 2048 samples = ~0.13s at 16kHz)
    bool showDebug = (debugCounter % 2048 == 0);
    
    if (bypassParam) {
        if (showDebug) {
            std::cout << "⏸️ BYPASSED | Sens=" << sensitivity << " | Gain=" << gain << "x" << std::endl;
        }
        return;
    }

    if (footstepClassifier == nullptr) {
        std::cerr << "❌ NULL CLASSIFIER!" << std::endl;
        return;
    }

    // Calculate buffer statistics
    float bufferRMS = 0.0f;
    float bufferMax = 0.0f;
    int samplesProcessed = 0;
    int detections_this_buffer = 0;
    
    // Professional gain smoothing
    static float currentGain = 1.0f;
    static float targetGain = 1.0f;
    const float gainSmoothingTime = 0.005f; // FASTER: 5ms smoothing
    const float gainSmoothingCoeff = 1.0f - std::exp(-1.0f / (getSampleRate() * gainSmoothingTime));
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            samplesProcessed++;
            
            if (std::isnan(inputSample) || std::isinf(inputSample)) {
                channelData[sample] = 0.0f;
                continue;
            }
            
            bufferRMS += inputSample * inputSample;
            bufferMax = std::max(bufferMax, std::abs(inputSample));
            
            // ENHANCED: Continuous footstep detection
            bool isFootstep = footstepClassifier->detectFootstep(inputSample, sensitivity);
            
            if (isFootstep) {
                detectionCount++;
                detections_this_buffer++;
                targetGain = gain;
                
                // IMMEDIATE output for every detection (no throttling)
                std::cout << "🎮 LIVE FOOTSTEP #" << detectionCount 
                          << " | Conf=" << footstepClassifier->getLastConfidence() 
                          << " | Energy=" << footstepClassifier->getLastEnergy() 
                          << " | Amp=" << gain << "x" << std::endl;
                std::cout.flush();
            } else {
                // Decay gain smoothly when no footstep
                targetGain = 1.0f + (targetGain - 1.0f) * 0.98f; // Slow decay
            }
            
            // ENHANCED: Much faster gain interpolation
            currentGain += (targetGain - currentGain) * gainSmoothingCoeff;
            
            // PROFESSIONAL: Multi-band amplification for significant effect
            float enhancedSample;
            if (currentGain > 1.1f) { // Apply enhancement when gain is significant
                enhancedSample = applyAdvancedAmplification(inputSample, currentGain, channel);
            } else {
                enhancedSample = inputSample * currentGain;
            }
            
            // Professional soft limiting
            if (std::abs(enhancedSample) > 0.85f) {
                float sign = (enhancedSample >= 0.0f) ? 1.0f : -1.0f;
                enhancedSample = sign * (0.85f + (std::abs(enhancedSample) - 0.85f) * 0.1f);
            }
            
            channelData[sample] = juce::jlimit(-1.0f, 1.0f, enhancedSample);
        }
    }
    
    if (samplesProcessed > 0) {
        bufferRMS = std::sqrt(bufferRMS / samplesProcessed);
    }
    
    // ENHANCED: High-frequency monitoring for client delivery
    if (showDebug) {
        std::cout << "🔊 LIVE MONITOR | RMS=" << std::fixed << std::setprecision(4) << bufferRMS 
                  << " | Max=" << bufferMax
                  << " | Sens=" << sensitivity 
                  << " | Gain=" << currentGain << "x"
                  << " | Detect/Buffer=" << detections_this_buffer
                  << " | Total=" << detectionCount << std::endl;
        
        if (footstepClassifier != nullptr) {
            std::cout << "🧠 CLASSIFIER | Conf=" << footstepClassifier->getLastConfidence()
                      << " | Energy=" << footstepClassifier->getLastEnergy()
                      << " | Noise=" << footstepClassifier->getBackgroundNoise()
                      << " | Cool=" << (footstepClassifier->isInCooldown() ? "Y" : "N") << std::endl;
        }
        
        std::cout.flush();
        
        if (debugFile.is_open()) {
            debugFile << "🔊 " << bufferRMS << " " << sensitivity << " " << currentGain << " " << detectionCount << std::endl;
            debugFile.flush();
        }
    }
}

// ENHANCED: Advanced multi-band amplification optimized for footstep range
float FootstepDetectorAudioProcessor::applyAdvancedAmplification(float inputSample, float gain, int channel)
{
    // Enhanced input validation
    if (std::isnan(inputSample) || std::isinf(inputSample) || channel < 0 || channel >= 2) {
        return inputSample;
    }
    
    float enhanced = 0.0f;
    
    // Apply enhanced frequency-specific amplification across 4 optimized bands
    for (int band = 0; band < 4; ++band) {
        float filteredSample = applyAmplificationFilter(inputSample, band, channel);
        
        // ENHANCED: Aggressive band-specific gains for maximum footstep enhancement
        float bandGain = 1.0f;
        switch (band) {
            case 0: bandGain = gain * 1.8f; break;  // 60-150Hz: High boost
            case 1: bandGain = gain * 2.2f; break;  // 150-300Hz: MAXIMUM boost
            case 2: bandGain = gain * 2.0f; break;  // 300-450Hz: Very high boost
            case 3: bandGain = gain * 1.4f; break;  // 450-600Hz: High boost
        }
        
        enhanced += filteredSample * bandGain;
    }
    
    // Apply enhanced dynamic compression for maximum audible effect
    enhanced = applyDynamicCompression(enhanced);
    
    // Enhanced mix ratio for obvious enhancement (90% processed, 10% original)
    return enhanced * 0.9f + inputSample * 0.1f;
}

// ENHANCED: Multi-band filters optimized for footstep frequencies
float FootstepDetectorAudioProcessor::applyAmplificationFilter(float sample, int bandIndex, int channel)
{
    // Enhanced bounds checking
    int filterIndex = channel * 4 + bandIndex;
    if (filterIndex < 0 || filterIndex >= amplificationFilters.size()) {
        return sample;
    }
    
    std::vector<float>& state = amplificationFilters[filterIndex];
    if (state.size() < 3) {
        return sample;
    }
    
    float output = 0.0f;
    
    // ENHANCED: Optimized IIR filters for footstep range
    switch (bandIndex) {
        case 0: // 60-150Hz (Low fundamentals)
            output = 0.35f * sample + 0.25f * state[0] - 0.1f * state[1] + 0.5f * state[2];
            break;
        case 1: // 150-300Hz (Primary footstep range)
            output = 0.5f * sample + 0.3f * state[0] - 0.15f * state[1] + 0.35f * state[2];
            break;
        case 2: // 300-450Hz (Footstep harmonics)
            output = 0.45f * sample + 0.25f * state[0] - 0.1f * state[1] + 0.4f * state[2];
            break;
        case 3: // 450-600Hz (Surface texture)
            output = 0.3f * sample + 0.15f * state[0] - 0.05f * state[1] + 0.6f * state[2];
            break;
    }
    
    // Enhanced filter state update
    state[1] = state[0];    // x[n-2] = x[n-1]
    state[0] = sample;      // x[n-1] = x[n]
    state[2] = output;      // y[n-1] = y[n]
    
    return output;
}

// ENHANCED: Aggressive dynamic compression for maximum audible enhancement
float FootstepDetectorAudioProcessor::applyDynamicCompression(float sample)
{
    float absValue = std::abs(sample);
    
    // Enhanced multi-stage aggressive compression for obvious enhancement
    if (absValue > 0.9f) {
        // Hard limiting above 0.9 to prevent clipping
        float compressed = 0.9f + (absValue - 0.9f) * 0.05f;
        return (sample >= 0.0f) ? compressed : -compressed;
    } else if (absValue > 0.7f) {
        // Strong compression between 0.7-0.9 for punch
        float compressed = 0.7f + (absValue - 0.7f) * 0.6f;
        return (sample >= 0.0f) ? compressed : -compressed;
    } else if (absValue > 0.4f) {
        // Medium compression between 0.4-0.7 for body
        float compressed = 0.4f + (absValue - 0.4f) * 0.8f;
        return (sample >= 0.0f) ? compressed : -compressed;
    }
    
    // Light boost below 0.4 to enhance quiet footsteps
    return sample * 1.1f;
}

bool FootstepDetectorAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FootstepDetectorAudioProcessor::createEditor()
{
    return new FootstepDetectorAudioProcessorEditor(*this);
}

void FootstepDetectorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Enhanced state saving with increased gain range
    juce::MemoryOutputStream mos(destData, true);
    mos.writeFloat(juce::jlimit(0.0f, 1.0f, sensitivityParam));
    mos.writeFloat(juce::jlimit(1.0f, 8.0f, gainParam)); // Enhanced range
    mos.writeBool(bypassParam);
}

void FootstepDetectorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Enhanced state loading with validation
    if (data == nullptr || sizeInBytes < 9) // Minimum size check
        return;
        
    juce::MemoryInputStream mis(data, static_cast<size_t>(sizeInBytes), false);
    
    if (mis.getNumBytesRemaining() >= 9) // 4+4+1 bytes
    {
        float newSensitivity = mis.readFloat();
        float newGain = mis.readFloat();
        bool newBypass = mis.readBool();
        
        // Enhanced validation and clamping
        sensitivityParam = juce::jlimit(0.0f, 1.0f, newSensitivity);
        gainParam = juce::jlimit(1.0f, 8.0f, newGain); // Enhanced range
        bypassParam = newBypass;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FootstepDetectorAudioProcessor();
}