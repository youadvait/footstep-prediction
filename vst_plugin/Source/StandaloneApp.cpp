#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

class FootstepDetectorStandaloneApp : public juce::JUCEApplication
{
public:
    FootstepDetectorStandaloneApp() = default;

    const juce::String getApplicationName() override { return "FootstepDetector"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        std::cout << "🚀 Starting MIDI-free FootstepDetector..." << std::endl;
        
        processor = std::make_unique<FootstepDetectorAudioProcessor>();
        
        deviceManager = std::make_unique<juce::AudioDeviceManager>();
        audioCallback = std::make_unique<AudioCallback>(processor.get());
        
        // Force specific audio settings
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        setup.inputChannels.setRange(0, 2, true);
        setup.outputChannels.setRange(0, 2, true);
        setup.sampleRate = 44100.0;
        setup.bufferSize = 512;
        setup.useDefaultInputChannels = true;
        setup.useDefaultOutputChannels = true;
        
        juce::String audioError = deviceManager->initialise(2, 2, nullptr, true, juce::String(), &setup);
        
        if (audioError.isNotEmpty()) {
            std::cout << "❌ Audio error: " << audioError.toStdString() << std::endl;
        } else {
            std::cout << "✅ Audio initialized WITH INPUT!" << std::endl;
        }
        
        auto* currentDevice = deviceManager->getCurrentAudioDevice();
        if (currentDevice != nullptr) {
            std::cout << "🎵 Device: " << currentDevice->getName().toStdString() << std::endl;
            std::cout << "🎵 Rate: " << currentDevice->getCurrentSampleRate() << "Hz" << std::endl;
            std::cout << "🎵 Buffer: " << currentDevice->getCurrentBufferSizeSamples() << std::endl;
            std::cout << "🎵 Inputs: " << currentDevice->getActiveInputChannels().toInteger() << std::endl;
        }
        
        deviceManager->addAudioCallback(audioCallback.get());
        
        if (currentDevice != nullptr) {
            processor->prepareToPlay(currentDevice->getCurrentSampleRate(), currentDevice->getCurrentBufferSizeSamples());
        }
        
        editor = std::unique_ptr<juce::AudioProcessorEditor>(processor->createEditor());
        if (editor != nullptr) {
            mainWindow = std::make_unique<MainWindow>("FootstepDetector Enhanced", editor.get());
        }
        
        std::cout << "✅ FootstepDetector ready - Audio processing ACTIVE!" << std::endl;
    }

    void shutdown() override
    {
        if (deviceManager != nullptr && audioCallback != nullptr) {
            deviceManager->removeAudioCallback(audioCallback.get());
            deviceManager->closeAudioDevice();
        }
        editor.reset();
        mainWindow.reset();
        processor.reset();
        audioCallback.reset();
        deviceManager.reset();
    }

    void systemRequestedQuit() override { quit(); }

private:
    class AudioCallback : public juce::AudioIODeviceCallback
    {
    public:
        AudioCallback(FootstepDetectorAudioProcessor* proc) : processor(proc) {}
        
        void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                             int numInputChannels,
                                             float* const* outputChannelData,
                                             int numOutputChannels,
                                             int numSamples,
                                             const juce::AudioIODeviceCallbackContext& context) override
        {
            // ENHANCED: Extensive debugging
            callbackCounter++;
            
            if (processor != nullptr) {
                // Create buffer for processing
                juce::AudioBuffer<float> buffer(numOutputChannels, numSamples);
                
                // CRITICAL: Copy input to buffer for processing
                for (int ch = 0; ch < juce::jmin(numInputChannels, numOutputChannels); ++ch) {
                    if (inputChannelData[ch] != nullptr) {
                        buffer.copyFrom(ch, 0, inputChannelData[ch], numSamples);
                    } else {
                        buffer.clear(ch, 0, numSamples);
                    }
                }
                
                // Calculate input RMS for debugging
                float inputRMS = 0.0f;
                if (numInputChannels > 0 && inputChannelData[0] != nullptr) {
                    for (int i = 0; i < numSamples; ++i) {
                        inputRMS += inputChannelData[0][i] * inputChannelData[0][i];
                    }
                    inputRMS = std::sqrt(inputRMS / numSamples);
                }
                
                // Debug every 0.5 seconds
                if (callbackCounter % 1100 == 0) { // ~0.5s at 44.1kHz/512
                    std::cout << "🔊 CALLBACK #" << callbackCounter 
                              << " | Input RMS: " << inputRMS
                              << " | Channels: " << numInputChannels 
                              << " | Samples: " << numSamples << std::endl;
                }
                
                // CRITICAL: Process through FootstepDetector
                juce::MidiBuffer midiBuffer;
                processor->processBlock(buffer, midiBuffer);
                
                // Copy processed buffer to output
                for (int ch = 0; ch < juce::jmin(numOutputChannels, buffer.getNumChannels()); ++ch) {
                    if (outputChannelData[ch] != nullptr) {
                        std::memcpy(outputChannelData[ch], buffer.getReadPointer(ch), numSamples * sizeof(float));
                    }
                }
            }
        }
        
        void audioDeviceAboutToStart(juce::AudioIODevice* device) override
        {
            std::cout << "🎵 Audio STARTING: " << device->getName().toStdString() << std::endl;
            callbackCounter = 0;
            if (processor != nullptr) {
                processor->prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
            }
        }
        
        void audioDeviceStopped() override
        {
            std::cout << "🎵 Audio STOPPED" << std::endl;
            if (processor != nullptr) {
                processor->releaseResources();
            }
        }
        
    private:
        FootstepDetectorAudioProcessor* processor;
        int callbackCounter = 0;
    };

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, juce::Component* content)
            : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(content, true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    std::unique_ptr<FootstepDetectorAudioProcessor> processor;
    std::unique_ptr<juce::AudioProcessorEditor> editor;
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    std::unique_ptr<AudioCallback> audioCallback;
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(FootstepDetectorStandaloneApp)