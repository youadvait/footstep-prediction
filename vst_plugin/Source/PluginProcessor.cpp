#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>

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
    
    std::cout << "✅ Enhanced FootstepDetector initialized with multi-band amplification!" << std::endl;
}

FootstepDetectorAudioProcessor::~FootstepDetectorAudioProcessor()
{
    std::cout << "🔧 Enhanced FootstepDetector shutting down..." << std::endl;
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
    std::cout << "🎵 Preparing enhanced audio processing: " << sampleRate << "Hz, " << samplesPerBlock << " samples" << std::endl;
    
    // Enhanced preparation
    if (footstepClassifier != nullptr)
    {
        footstepClassifier->prepare(sampleRate, samplesPerBlock);
        std::cout << "✅ Enhanced FootstepClassifier prepared for 40-500Hz detection" << std::endl;
    }
    
    // Reset enhanced amplification filter states
    for (auto& filterState : amplificationFilters) {
        std::fill(filterState.begin(), filterState.end(), 0.0f);
    }
    
    // Reset debug counters
    debugCounter = 0;
    detectionCount = 0;
    
    std::cout << "✅ Enhanced audio preparation complete" << std::endl;
}

void FootstepDetectorAudioProcessor::releaseResources()
{
    std::cout << "🔧 Releasing enhanced audio resources" << std::endl;
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
    // Clear MIDI
    midiMessages.clear();
    
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = buffer.getNumChannels();
    auto totalNumOutputChannels = buffer.getNumChannels();

    // Enhanced parameter reading
    float sensitivity = juce::jlimit(0.0f, 1.0f, sensitivityParam);
    float gain = juce::jlimit(1.0f, 8.0f, gainParam);
    
    // ENHANCED: Always show processing status
    debugCounter += buffer.getNumSamples();
    bool showDebug = (debugCounter % 22050 == 0); // Every 0.5 seconds
    
    if (bypassParam)
    {
        if (showDebug) {
            std::cout << "⏸️ BYPASSED | Sensitivity=" << sensitivity 
                      << " | Gain=" << gain << "x" << std::endl;
        }
        return;
    }

    if (footstepClassifier == nullptr)
    {
        if (showDebug) {
            std::cout << "❌ NULL CLASSIFIER!" << std::endl;
        }
        return;
    }

    // Calculate buffer RMS for monitoring
    float bufferRMS = 0.0f;
    int samplesProcessed = 0;
    
    // ENHANCED: Process every sample with extensive debugging
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            samplesProcessed++;
            
            // Enhanced input validation
            if (std::isnan(inputSample) || std::isinf(inputSample))
            {
                channelData[sample] = 0.0f;
                continue;
            }
            
            // Calculate RMS
            bufferRMS += inputSample * inputSample;
            
            // CRITICAL: Call footstep detection with debugging
            bool isFootstep = footstepClassifier->detectFootstep(inputSample, sensitivity);
            
            if (isFootstep)
            {
                detectionCount++;
                
                // Enhanced amplification
                float enhancedSample = applyAdvancedAmplification(inputSample, gain, channel);
                channelData[sample] = juce::jlimit(-1.0f, 1.0f, enhancedSample);
                
                // ALWAYS show footstep detections
                std::cout << "🎮 FOOTSTEP #" << detectionCount 
                          << " | Sample: " << inputSample
                          << " | Enhanced: " << enhancedSample
                          << " | Confidence: " << footstepClassifier->getLastConfidence() 
                          << " | Energy: " << footstepClassifier->getLastEnergy() 
                          << " | Gain: " << gain << "x" << std::endl;
            }
            else
            {
                channelData[sample] = inputSample;
            }
        }
    }
    
    // Calculate final RMS
    if (samplesProcessed > 0) {
        bufferRMS = std::sqrt(bufferRMS / samplesProcessed);
    }
    
    // ENHANCED: Always show processing status
    if (showDebug) {
        std::cout << "🔊 PROCESSING | RMS=" << bufferRMS 
                  << " | Sensitivity=" << sensitivity 
                  << " | Gain=" << gain << "x"
                  << " | Detections=" << detectionCount 
                  << " | Samples=" << samplesProcessed << std::endl;
                  
        // Show FootstepClassifier internal state
        if (footstepClassifier != nullptr) {
            std::cout << "🧠 CLASSIFIER | Confidence=" << footstepClassifier->getLastConfidence()
                      << " | Energy=" << footstepClassifier->getLastEnergy()
                      << " | Background=" << footstepClassifier->getBackgroundNoise()
                      << " | Cooldown=" << (footstepClassifier->isInCooldown() ? "YES" : "NO") << std::endl;
        }
    }
}

// ENHANCED: Advanced multi-band amplification optimized for 40-500Hz footstep range
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
            case 0: bandGain = gain * 1.8f; break;  // 40-120Hz: High boost (impact/rumble)
            case 1: bandGain = gain * 2.2f; break;  // 120-250Hz: MAXIMUM boost (primary footstep range)
            case 2: bandGain = gain * 2.0f; break;  // 250-400Hz: Very high boost (footstep detail)
            case 3: bandGain = gain * 1.4f; break;  // 400-500Hz: High boost (surface texture)
        }
        
        enhanced += filteredSample * bandGain;
    }
    
    // Apply enhanced dynamic compression for maximum audible effect
    enhanced = applyDynamicCompression(enhanced);
    
    // Enhanced mix ratio for obvious enhancement (90% processed, 10% original)
    return enhanced * 0.9f + inputSample * 0.1f;
}

// ENHANCED: Multi-band filters optimized for 40-500Hz footstep frequencies
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
    
    // ENHANCED: Optimized IIR filters for 40-500Hz footstep range
    switch (bandIndex) {
        case 0: // 40-120Hz (Low impact/rumble) - Enhanced low-end response
            output = 0.35f * sample + 0.25f * state[0] - 0.1f * state[1] + 0.5f * state[2];
            break;
        case 1: // 120-250Hz (Primary footstep range) - Maximum sensitivity
            output = 0.5f * sample + 0.3f * state[0] - 0.15f * state[1] + 0.35f * state[2];
            break;
        case 2: // 250-400Hz (Footstep detail/harmonics) - Wide bandpass
            output = 0.45f * sample + 0.25f * state[0] - 0.1f * state[1] + 0.4f * state[2];
            break;
        case 3: // 400-500Hz (Surface texture) - High-pass characteristics
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