#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessor::FootstepDetectorAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       parameters (*this, nullptr, juce::Identifier("FootstepDetector"),
                   {
                       std::make_unique<juce::AudioParameterFloat>("sensitivity",
                                                                   "Sensitivity",
                                                                   juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                                                                   0.5f),
                       std::make_unique<juce::AudioParameterFloat>("gain",
                                                                   "Gain",
                                                                   juce::NormalisableRange<float>(1.0f, 5.0f, 0.1f),
                                                                   3.0f),
                       std::make_unique<juce::AudioParameterBool>("bypass",
                                                                  "Bypass",
                                                                  false)
                   }),
       footstepClassifier(std::make_unique<FootstepClassifier>())
{
    // CRITICAL FIX: Thread-safe parameter initialization
    sensitivityParam = parameters.getRawParameterValue("sensitivity");
    gainParam = parameters.getRawParameterValue("gain");
    bypassParam = parameters.getRawParameterValue("bypass");
}

FootstepDetectorAudioProcessor::~FootstepDetectorAudioProcessor()
{
}

void FootstepDetectorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    footstepClassifier->prepare(sampleRate, samplesPerBlock);
}

void FootstepDetectorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // CRITICAL FIX: Safe parameter reading with null checks
    if (sensitivityParam == nullptr || gainParam == nullptr || bypassParam == nullptr)
        return;
    
    // Thread-safe parameter access
    float sensitivity = sensitivityParam->load();
    float gain = gainParam->load();
    bool bypass = bypassParam->load() > 0.5f;
    
    // CRITICAL FIX: Parameter validation
    sensitivity = juce::jlimit(0.0f, 1.0f, sensitivity);
    gain = juce::jlimit(1.0f, 5.0f, gain);
    
    if (bypass)
    {
        return; // Pass through unprocessed
    }
    
    // Safe audio processing
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Process with validated parameters
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            
            // SAFE footstep detection
            try {
                bool isFootstep = footstepClassifier->detectFootstep(inputSample, sensitivity);
                
                if (isFootstep)
                {
                    // SAFE amplification with bounds checking
                    float amplified = inputSample * gain;
                    channelData[sample] = juce::jlimit(-1.0f, 1.0f, amplified);
                }
                else
                {
                    channelData[sample] = inputSample;
                }
            }
            catch (...)
            {
                // Fallback: pass through unprocessed on any error
                channelData[sample] = inputSample;
            }
        }
    }
}

// CRITICAL FIX: Safe parameter change handling
void FootstepDetectorAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Validate and clamp all parameter changes
    if (parameterID == "sensitivity")
    {
        newValue = juce::jlimit(0.0f, 1.0f, newValue);
    }
    else if (parameterID == "gain")
    {
        newValue = juce::jlimit(1.0f, 5.0f, newValue);
    }
    else if (parameterID == "bypass")
    {
        newValue = (newValue > 0.5f) ? 1.0f : 0.0f;
    }
}

bool FootstepDetectorAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FootstepDetectorAudioProcessor::createEditor()
{
    return new FootstepDetectorAudioProcessorEditor (*this, parameters);
}

void FootstepDetectorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void FootstepDetectorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FootstepDetectorAudioProcessor();
}
