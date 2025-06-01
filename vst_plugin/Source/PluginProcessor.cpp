#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorProcessor::FootstepDetectorProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    std::cout << "🎯 FINAL FootstepDetector - Energy + Frequency Analysis!" << std::endl;
    
    // Initialize ONLY the classifier (no MFCC needed)
    footstepClassifier = std::make_unique<FootstepClassifier>();
    
    initializeBuffers();
    
    std::cout << "✅ FINAL FootstepDetector initialized" << std::endl;
}

FootstepDetectorProcessor::~FootstepDetectorProcessor()
{
}

void FootstepDetectorProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    std::cout << "🔧 Preparing FINAL audio: " << sampleRate << "Hz, " << samplesPerBlock << " samples" << std::endl;
    
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    initializeBuffers();
    
    std::cout << "✅ FINAL version prepared for STEREO processing" << std::endl;
}

void FootstepDetectorProcessor::releaseResources()
{
}

void FootstepDetectorProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // Clear unused channels
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // Bypass if requested
    if (bypassed.load()) {
        return;
    }

    // DIRECT AUDIO ANALYSIS (no MFCC buffer needed)
    if (numSamples > 0 && numChannels > 0)
    {
        const float* leftChannelData = buffer.getReadPointer(0);
        performDirectAudioAnalysis(leftChannelData, numSamples);
    }

    float confidence = currentConfidence.load();
    
    // Copy left to right for stereo output
    if (numChannels == 2 && totalNumInputChannels == 1)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
    }
    
    // Apply footstep amplification
    applyFootstepAmplification(buffer, confidence);

    // Debug output (less frequent)
    static int audioDebugCounter = 0;
    if (++audioDebugCounter % 200 == 0)
    {
        float rms = buffer.getRMSLevel(0, 0, numSamples);
        std::cout << "🎵 Buffer RMS: " << std::fixed << std::setprecision(4) << rms 
                  << ", Confidence: " << std::setprecision(3) << confidence;
        
        if (confidence > 0.7f) {
            std::cout << " 🔥 FOOTSTEP BOOST";
        }
        
        std::cout << std::endl;
    }
}

void FootstepDetectorProcessor::performDirectAudioAnalysis(const float* audioData, int numSamples)
{
    if (!footstepClassifier) return;
    
    try {
        // DIRECT analysis - no MFCC extraction needed
        float mlConfidence = footstepClassifier->classifyFootstep(
            audioData, numSamples, static_cast<float>(currentSampleRate)
        );
        
        // Apply sensitivity adjustment
        float sensitivityValue = sensitivity.load();
        float adjustedConfidence = mlConfidence * (0.9f + sensitivityValue * 0.2f);
        
        // Store confidence
        currentConfidence.store(adjustedConfidence);
        
        // Detection output with final threshold
        if (mlConfidence > 0.7f) {
            static int detectionCount = 0;
            if (++detectionCount % 2 == 0) {
                std::cout << "🎮 FINAL FOOTSTEP DETECTED: " << std::fixed << std::setprecision(3) 
                          << mlConfidence << " ⭐" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Final detection error: " << e.what() << std::endl;
    }
}

void FootstepDetectorProcessor::applyFootstepAmplification(juce::AudioBuffer<float>& buffer, float confidence)
{
    if (confidence <= 0.7f) return; // Final threshold for amplification
    
    float gainValue = footstepGain.load();
    float amplificationFactor = 1.0f + (gainValue - 1.0f) * confidence;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= amplificationFactor;
            
            // Soft limiting to prevent clipping
            channelData[sample] = std::tanh(channelData[sample] * 0.8f);
        }
    }
}

void FootstepDetectorProcessor::initializeBuffers()
{
    analysisBuffer.resize(ANALYSIS_WINDOW_SIZE, 0.0f);
    energyHistory.resize(ENERGY_HISTORY_SIZE, 0.0f);
    
    analysisBufferPos = 0;
    bufferWritePosition = 0;
    
    std::cout << "📊 FINAL buffers ready - Direct analysis mode" << std::endl;
}

juce::AudioProcessorEditor* FootstepDetectorProcessor::createEditor()
{
    return new FootstepDetectorEditor (*this);
}

void FootstepDetectorProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
}

void FootstepDetectorProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FootstepDetectorProcessor();
}