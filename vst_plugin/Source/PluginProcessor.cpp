#include "PluginProcessor.h"
#include "PluginEditor.h"

FootstepDetectorAudioProcessor::FootstepDetectorAudioProcessor()
{
    footstepClassifier = std::make_unique<FootstepClassifier>();
}

FootstepDetectorAudioProcessor::~FootstepDetectorAudioProcessor()
{
}

const juce::String FootstepDetectorAudioProcessor::getName() const
{
    return JucePlugin_Name;
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
    footstepClassifier->prepare(sampleRate, samplesPerBlock);
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

    // BASIC SAFETY: Simple parameter bounds checking
    float sensitivity = juce::jlimit(0.0f, 1.0f, sensitivityParam);
    float gain = juce::jlimit(1.0f, 5.0f, gainParam);
    
    if (bypassParam)
    {
        return; // Pass through unchanged
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            
            // SAFE processing with try-catch
            try {
                bool isFootstep = footstepClassifier->detectFootstep(inputSample, sensitivity);
                
                if (isFootstep)
                {
                    // SAFE amplification with clipping protection
                    float amplified = inputSample * gain;
                    channelData[sample] = juce::jlimit(-1.0f, 1.0f, amplified);
                }
                else
                {
                    channelData[sample] = inputSample;
                }
            }
            catch (...) {
                // Fallback: pass through on any error
                channelData[sample] = inputSample;
            }
        }
    }
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
    juce::MemoryOutputStream mos(destData, true);
    mos.writeFloat(sensitivityParam);
    mos.writeFloat(gainParam);
    mos.writeBool(bypassParam);
}

void FootstepDetectorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream mis(data, static_cast<size_t>(sizeInBytes), false);
    
    // SAFE parameter loading with validation
    if (mis.getNumBytesRemaining() >= 9) // 4+4+1 bytes minimum
    {
        float newSensitivity = mis.readFloat();
        float newGain = mis.readFloat();
        bool newBypass = mis.readBool();
        
        // Validate and clamp loaded values
        sensitivityParam = juce::jlimit(0.0f, 1.0f, newSensitivity);
        gainParam = juce::jlimit(1.0f, 5.0f, newGain);
        bypassParam = newBypass;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FootstepDetectorAudioProcessor();
}
