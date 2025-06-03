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
        
        // FORCE 44.1kHz for better frequency analysis
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        setup.inputChannels.setRange(0, 2, true);
        setup.outputChannels.setRange(0, 2, true);
        setup.sampleRate = 44100.0; // FORCE higher sample rate
        setup.bufferSize = 256;     // SMALLER buffer for lower latency
        setup.useDefaultInputChannels = true;
        setup.useDefaultOutputChannels = true;
        
        juce::String audioError = deviceManager->initialise(2, 2, nullptr, true, juce::String(), &setup);
        
        if (audioError.isNotEmpty()) {
            std::cout << "❌ Audio error: " << audioError.toStdString() << std::endl;
            // Try fallback settings
            audioError = deviceManager->initialise(2, 2, nullptr, true);
        } else {
            std::cout << "✅ Audio initialized WITH INPUT!" << std::endl;
        }
        
        auto* currentDevice = deviceManager->getCurrentAudioDevice();
        if (currentDevice != nullptr) {
            std::cout << "🎵 Device: " << currentDevice->getName().toStdString() << std::endl;
            std::cout << "🎵 Rate: " << currentDevice->getCurrentSampleRate() << "Hz" << std::endl;
            std::cout << "🎵 Buffer: " << currentDevice->getCurrentBufferSizeSamples() << std::endl;
            std::cout << "🎵 Inputs: " << currentDevice->getActiveInputChannels().toInteger() << std::endl;
            
            // FORCE sample rate change if not 44.1kHz
            if (currentDevice->getCurrentSampleRate() != 44100.0) {
                std::cout << "⚠️ WARNING: Not running at 44.1kHz - detection may be suboptimal" << std::endl;
            }
        }
        
        deviceManager->addAudioCallback(audioCallback.get());
        
        if (currentDevice != nullptr) {
            processor->prepareToPlay(currentDevice->getCurrentSampleRate(), currentDevice->getCurrentBufferSizeSamples());
        }
        
        editor = std::unique_ptr<juce::AudioProcessorEditor>(processor->createEditor());
        if (editor != nullptr) {
            mainWindow = std::make_unique<MainWindow>("FootstepDetector - CLIENT DELIVERY", editor.get());
        }
        
        std::cout << "✅ FootstepDetector ready - HIGH-PERFORMANCE MODE!" << std::endl;
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
            callbackCounter++;
            
            if (processor != nullptr) {
                juce::AudioBuffer<float> buffer(numOutputChannels, numSamples);
                
                // Copy input to buffer
                for (int ch = 0; ch < juce::jmin(numInputChannels, numOutputChannels); ++ch) {
                    if (inputChannelData[ch] != nullptr) {
                        buffer.copyFrom(ch, 0, inputChannelData[ch], numSamples);
                    } else {
                        buffer.clear(ch, 0, numSamples);
                    }
                }
                
                // Calculate input statistics
                float inputRMS = 0.0f;
                float inputMax = 0.0f;
                if (numInputChannels > 0 && inputChannelData[0] != nullptr) {
                    for (int i = 0; i < numSamples; ++i) {
                        float sample = inputChannelData[0][i];
                        inputRMS += sample * sample;
                        inputMax = std::max(inputMax, std::abs(sample));
                    }
                    inputRMS = std::sqrt(inputRMS / numSamples);
                }
                
                // ENHANCED: More frequent callback monitoring (every 200 callbacks instead of 1100)
                if (callbackCounter % 200 == 0) {
                    std::cout << "🔊 CALLBACK #" << callbackCounter 
                            << " | Input RMS=" << std::fixed << std::setprecision(4) << inputRMS
                            << " | Max=" << inputMax
                            << " | Ch=" << numInputChannels 
                            << " | Samp=" << numSamples << std::endl;
                    std::cout.flush();
                }
                
                // Process through FootstepDetector
                juce::MidiBuffer midiBuffer;
                processor->processBlock(buffer, midiBuffer);
                
                // Copy to output
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