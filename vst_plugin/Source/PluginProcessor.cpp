#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessor::FootstepDetectorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                     ),
#else
    :
#endif
    parameters (*this, nullptr, juce::Identifier ("FootstepDetector"),
                {
                    std::make_unique<juce::AudioParameterFloat> ("sensitivity", "Sensitivity", 0.0f, 1.0f, 0.5f),
                    std::make_unique<juce::AudioParameterFloat> ("gain", "Gain", 1.0f, 8.0f, 3.0f),
                    std::make_unique<juce::AudioParameterBool> ("bypass", "Bypass", false)
                })
{
    footstepClassifier = std::make_unique<FootstepClassifier>();
    
    // Initialize simple EQ filters for stereo
    lowShelfFilter.resize(2);   // 180Hz band
    midShelfFilter.resize(2);   // 300Hz band  
    highShelfFilter.resize(2);  // 450Hz band
    
    for (auto& filter : lowShelfFilter) {
        filter.reset();
    }
    for (auto& filter : midShelfFilter) {
        filter.reset();
    }
    for (auto& filter : highShelfFilter) {
        filter.reset();
    }
    
    // Get parameter pointers
    sensitivityParam = parameters.getRawParameterValue ("sensitivity");
    gainParam = parameters.getRawParameterValue ("gain");
    bypassParam = parameters.getRawParameterValue ("bypass");
}

FootstepDetectorAudioProcessor::~FootstepDetectorAudioProcessor()
{
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
    // Simple preparation
    if (footstepClassifier != nullptr)
    {
        footstepClassifier->prepare(sampleRate, samplesPerBlock);
    }
    
    // Setup EQ filters for footstep frequency enhancement
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    
    for (auto& filter : lowShelfFilter) {
        filter.prepare(spec);
        filter.reset();
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate,
            180.0f,  // Low footstep frequencies
            0.8f,    // Q factor
            25.0f    // 18dB gain for obvious amplification
        );
    }
    
    for (auto& filter : midShelfFilter) {
        filter.prepare(spec);
        filter.reset();
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            300.0f,  // Mid footstep frequencies
            0.7f,    // Q factor
            22.0f    // 15dB gain
        );
    }
    
    for (auto& filter : highShelfFilter) {
        filter.prepare(spec);
        filter.reset();
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            450.0f,  // High footstep frequencies
            0.6f,    // Q factor
            18.0f    // 12dB gain
        );
    }
}

void FootstepDetectorAudioProcessor::getEditorSize(int& width, int& height)
{
    // Provide default size to prevent EqualizerAPO crash
    width = 400;
    height = 300;
}

juce::AudioProcessorEditor* FootstepDetectorAudioProcessor::createEditor()
{
    // Return generic editor for EqualizerAPO compatibility
    return new juce::GenericAudioProcessorEditor(*this);
}

void FootstepDetectorAudioProcessor::releaseResources()
{
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
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // FIXED: Use atomic parameter values from AudioProcessorValueTreeState
    float sensitivity = juce::jlimit(0.0f, 1.0f, sensitivityParam->load());
    float gain = juce::jlimit(1.0f, 8.0f, gainParam->load());
    bool bypass = bypassParam->load() > 0.5f;
    
    if (bypass)
    {
        return; // Pass through unchanged
    }

    if (footstepClassifier == nullptr)
    {
        return; // Pass through if classifier not available
    }

    // Simple approach: Process sample by sample with detection + EQ
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            
            if (std::isnan(inputSample) || std::isinf(inputSample))
            {
                channelData[sample] = 0.0f;
                continue;
            }
            
            // Simple footstep detection (using proven working logic)
            bool isFootstep = footstepClassifier->detectFootstep(inputSample, sensitivity);
            
            if (isFootstep)
            {
                // ENHANCED: Apply multi-band EQ + gain + saturation for massive enhancement
                float multiBandSample = applyMultiBandEQ(inputSample, channel);
                float amplified = multiBandSample * gain;
                
                // ADD: Aggressive saturation for maximum audible effect
                amplified = applySaturation(amplified);
                
                channelData[sample] = juce::jlimit(-1.0f, 1.0f, amplified);
            }
            
            else
            {
                // ALWAYS apply subtle EQ for footstep frequency enhancement
                channelData[sample] = inputSample; // Pass through unchanged
            }
        }
    }
}

// ADD: Aggressive saturation for maximum footstep enhancement
float FootstepDetectorAudioProcessor::applySaturation(float sample)
{
    // Soft saturation for harmonics and perceived loudness
    float saturated = sample;
    
    if (std::abs(sample) > 0.3f) {
        float sign = (sample >= 0.0f) ? 1.0f : -1.0f;
        float abs_sample = std::abs(sample);
        
        // Aggressive saturation curve
        saturated = sign * (0.3f + (abs_sample - 0.3f) * 0.4f + 
                           std::sin((abs_sample - 0.3f) * 3.14159f) * 0.1f);
    }
    
    // Additional harmonic enhancement
    saturated += std::sin(sample * 6.28318f) * 0.05f; // 2nd harmonic
    
    return saturated * 1.5f; // Extra boost
}


// Simple EQ-based footstep enhancement
float FootstepDetectorAudioProcessor::applyFootstepEQ(float sample, int channel)
{
    if (channel < 0 || channel >= lowShelfFilter.size()) {
        return sample;
    }
    
    // Apply low shelf filter to boost footstep frequencies (50-250Hz)
    float eqSample = lowShelfFilter[channel].processSample(sample);
    
    return eqSample;
}

// ENHANCED: Multi-band footstep enhancement
float FootstepDetectorAudioProcessor::applyMultiBandEQ(float sample, int channel)
{
    if (channel < 0 || channel >= lowShelfFilter.size()) {
        return sample;
    }
    
    // Apply multi-band processing for full footstep spectrum enhancement
    float lowBand = lowShelfFilter[channel].processSample(sample);    // 180Hz boost
    float midBand = midShelfFilter[channel].processSample(sample);    // 300Hz boost
    float highBand = highShelfFilter[channel].processSample(sample);  // 450Hz boost
    
    // Combine bands with weighted mixing for natural sound
    float enhanced = (lowBand * 0.6f) + (midBand * 0.5f) + (highBand * 0.4f);
    
    return enhanced;
}

bool FootstepDetectorAudioProcessor::hasEditor() const
{
    return true;
}

void FootstepDetectorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // FIXED: Use AudioProcessorValueTreeState for proper state management
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FootstepDetectorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // FIXED: Use AudioProcessorValueTreeState for proper state management
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// ADD THESE MISSING PARAMETER METHODS:

float FootstepDetectorAudioProcessor::getParameter(int index)
{
    switch (index) {
        case 0: return sensitivityParam->load();
        case 1: return (gainParam->load() - 1.0f) / 7.0f; // Normalize 1-8 to 0-1
        case 2: return bypassParam->load();
        default: return 0.0f;
    }
}

void FootstepDetectorAudioProcessor::setParameter(int index, float value)
{
    switch (index) {
        case 0: sensitivityParam->store(juce::jlimit(0.0f, 1.0f, value)); break;
        case 1: gainParam->store(1.0f + (juce::jlimit(0.0f, 1.0f, value) * 7.0f)); break;
        case 2: bypassParam->store(value > 0.5f ? 1.0f : 0.0f); break;
    }
}

const juce::String FootstepDetectorAudioProcessor::getParameterName(int index)
{
    switch (index) {
        case 0: return "Sensitivity";
        case 1: return "Gain";
        case 2: return "Bypass";
        default: return {};
    }
}

const juce::String FootstepDetectorAudioProcessor::getParameterText(int index)
{
    switch (index) {
        case 0: return juce::String(sensitivityParam->load(), 2);
        case 1: return juce::String(gainParam->load(), 1) + "x";
        case 2: return bypassParam->load() > 0.5f ? "On" : "Off";
        default: return {};
    }
}

// ADD: EqualizerAPO compatibility method
void FootstepDetectorAudioProcessor::getEditorSize(int& width, int& height)
{
    width = 400;
    height = 300;
}

namespace juce
{
    float JUCE_CALLTYPE handleManufacturerSpecificVST2Opcode (int, long long, void*, float)
    {
        return 0.0f; // Return 0 for unsupported opcodes
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FootstepDetectorAudioProcessor();
}
