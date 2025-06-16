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
        std::make_unique<juce::AudioParameterFloat> ("sensitivity", "Sensitivity", 0.0f, 1.0f, 0.7f),
        std::make_unique<juce::AudioParameterFloat> ("reduction", "Reduction", 0.1f, 0.8f, 0.3f),
        std::make_unique<juce::AudioParameterFloat> ("enhancement", "Enhancement", 1.0f, 2.0f, 1.2f),
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
    reductionParam = parameters.getRawParameterValue ("reduction");
    enhancementParam = parameters.getRawParameterValue ("enhancement");
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
    (void)index;
}

const juce::String FootstepDetectorAudioProcessor::getProgramName(int index)
{
    (void)index;
    return {};
}

void FootstepDetectorAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    (void)index;
    (void)newName;
}

void FootstepDetectorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (footstepClassifier != nullptr)
    {
        footstepClassifier->prepare(sampleRate, samplesPerBlock);
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;

    footstepHoldDuration = static_cast<int>(sampleRate * 0.2);
    
    for (auto& filter : lowShelfFilter) {
        filter.prepare(spec);
        filter.reset();
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate,
            180.0f,
            0.8f,
            3.5f     // ULTRA-CONSERVATIVE: 2dB = 1.26x (was 6dB!)
        );
    }

    for (auto& filter : midShelfFilter) {
        filter.prepare(spec);
        filter.reset();
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            300.0f,
            0.7f,
            3.0f     // ULTRA-CONSERVATIVE: 1.5dB = 1.19x (was 5dB!)
        );
    }

    for (auto& filter : highShelfFilter) {
        filter.prepare(spec);
        filter.reset();
        filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate,
            450.0f,
            0.6f,
            2.5f     // ULTRA-CONSERVATIVE: 1dB = 1.12x (was 4dB!)
        );
    }

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
    juce::ScopedLock lock(processingLock);
    
    if (isProcessing) return;
    isProcessing = true;
    
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float sensitivity = juce::jlimit(0.0f, 1.0f, sensitivityParam->load());
    float reduction = juce::jlimit(0.1f, 0.8f, reductionParam->load());        // How much to reduce non-footsteps
    float enhancement = juce::jlimit(1.0f, 2.0f, enhancementParam->load());   // How much to enhance footsteps
    bool bypass = bypassParam->load() > 0.5f;
    
    if (bypass) {
        isProcessing = false;
        return;
    }

    if (footstepClassifier == nullptr) {
        isProcessing = false;
        return;
    }

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
            
            // Detection with balanced threshold (same as before)
            bool isFootstep = footstepClassifier->detectFootstep(inputSample, sensitivity);
            
            // REVERSED LOGIC: Reduce non-footsteps, enhance footsteps
            if (isFootstep)
            {
                // FOOTSTEP DETECTED: Keep at normal volume or slightly enhance
                targetAmplification = enhancement; // 1.0 - 2.0x (slight enhancement)
                holdSamples = footstepHoldDuration; // 200ms hold
                inHoldPhase = true;
            }
            else if (inHoldPhase && holdSamples > 0)
            {
                // HOLD PHASE: Continue enhancing footstep
                targetAmplification = enhancement;
                holdSamples--; // Count down hold time
            }
            else if (inHoldPhase && holdSamples <= 0)
            {
                // END HOLD: Start reducing non-footstep sounds
                targetAmplification = reduction; // 0.1 - 0.8x (reduce non-footsteps)
                inHoldPhase = false;
            }
            else
            {
                // NOT FOOTSTEP: Reduce volume of everything else
                targetAmplification = reduction; // 0.1 - 0.8x (smart noise reduction)
            }
            
            // Smooth envelope interpolation (same as before)
            if (currentAmplification < targetAmplification)
            {
                // Attack phase
                currentAmplification += (targetAmplification - currentAmplification) * envelopeAttack;
            }
            else if (currentAmplification > targetAmplification)
            {
                // Release phase  
                currentAmplification += (targetAmplification - currentAmplification) * envelopeRelease;
            }
            
            // Apply smart noise reduction processing
            if (std::abs(currentAmplification - 1.0f) > 0.01f) // Only process if not unity gain
            {
                float processedSample = inputSample;
                
                // Apply EQ enhancement only when amplifying footsteps
                if (currentAmplification > 1.0f) {
                    processedSample = applyMultiBandEQ(inputSample, channel);
                }
                
                float amplified = processedSample * currentAmplification;
                
                // Gentle limiting for both enhancement and reduction
                if (std::abs(amplified) > 0.8f) {
                    float sign = (amplified >= 0.0f) ? 1.0f : -1.0f;
                    float abs_amp = std::abs(amplified);
                    amplified = sign * (0.8f + (abs_amp - 0.8f) * 0.1f);
                }
                
                channelData[sample] = juce::jlimit(-0.9f, 0.9f, amplified);
            }
            else
            {
                channelData[sample] = inputSample; // Pass through unchanged
            }
        }
    }
    
    isProcessing = false;
}


// float FootstepDetectorAudioProcessor::applySaturation(float sample)
// {
//     // FIXED: Gentle soft limiting instead of aggressive saturation
//     float limited = sample;
    
//     // Soft knee compression above 0.7
//     if (std::abs(sample) > 0.7f) {
//         float sign = (sample >= 0.0f) ? 1.0f : -1.0f;
//         float abs_sample = std::abs(sample);
        
//         // Gentle soft limiting curve
//         limited = sign * (0.7f + (abs_sample - 0.7f) * 0.3f);
//     }
    
//     // REMOVED: Extra 1.5x boost that was causing clipping
//     return juce::jlimit(-0.95f, 0.95f, limited); // Hard limit at ±0.95
// }


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

float FootstepDetectorAudioProcessor::applyMultiBandEQ(float sample, int channel)
{
    if (channel < 0 || channel >= lowShelfFilter.size()) {
        return sample;
    }
    
    // Apply conservative multi-band processing
    float lowBand = lowShelfFilter[channel].processSample(sample);
    float midBand = midShelfFilter[channel].processSample(sample);
    float highBand = highShelfFilter[channel].processSample(sample);
    
    // CLEAN mixing - unity gain total
    float enhanced = (lowBand * 0.4f) + (midBand * 0.35f) + (highBand * 0.25f);
    
    // NO extra boost - clean processing only
    return enhanced; // Total gain ~1.0x from EQ
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
        case 1: return (reductionParam->load() - 0.1f) / 0.7f; // Normalize 0.1-0.8 to 0-1
        case 2: return (enhancementParam->load() - 1.0f) / 1.0f; // Normalize 1.0-2.0 to 0-1
        case 3: return bypassParam->load();
        default: return 0.0f;
    }
}

void FootstepDetectorAudioProcessor::setParameter(int index, float value)
{
    switch (index) {
        case 0: sensitivityParam->store(juce::jlimit(0.0f, 1.0f, value)); break;
        case 1: reductionParam->store(0.1f + (juce::jlimit(0.0f, 1.0f, value) * 0.7f)); break;
        case 2: enhancementParam->store(1.0f + (juce::jlimit(0.0f, 1.0f, value) * 1.0f)); break;
        case 3: bypassParam->store(value > 0.5f ? 1.0f : 0.0f); break;
    }
}

const juce::String FootstepDetectorAudioProcessor::getParameterName(int index)
{
    switch (index) {
        case 0: return "Sensitivity";
        case 1: return "Reduction";
        case 2: return "Enhancement";
        case 3: return "Bypass";
        default: return {};
    }
}

const juce::String FootstepDetectorAudioProcessor::getParameterText(int index)
{
    switch (index) {
        case 0: return juce::String(sensitivityParam->load(), 2);
        case 1: return juce::String((1.0f - reductionParam->load()) * 100.0f, 0) + "%"; // Show as % reduction
        case 2: return juce::String(enhancementParam->load(), 1) + "x";
        case 3: return bypassParam->load() > 0.5f ? "On" : "Off";
        default: return {};
    }
}

int FootstepDetectorAudioProcessor::getNumParameters() override { return 4; } // Now 4 parameters

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
