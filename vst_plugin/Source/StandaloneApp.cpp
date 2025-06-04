#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

// NUCLEAR: Completely bypass JUCE's MIDI-enabled standalone wrapper
class MIDIFreeStandaloneApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "FootstepDetector"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        std::cout << "🚀 MIDI-FREE FootstepDetector Starting..." << std::endl;
        
        processor = std::make_unique<FootstepDetectorAudioProcessor>();
        
        // CRITICAL: Initialize AudioDeviceManager WITHOUT any MIDI enumeration
        deviceManager = std::make_unique<juce::AudioDeviceManager>();
        
        // BYPASS: Skip all MIDI initialization completely
        juce::String error = deviceManager->initialiseWithDefaultDevices(2, 2);
        
        if (error.isEmpty()) {
            std::cout << "✅ Audio initialized - MIDI COMPLETELY BYPASSED!" << std::endl;
        } else {
            std::cout << "❌ Audio error: " << error.toStdString() << std::endl;
        }
        
        // Set up audio callback
        deviceManager->addAudioCallback(&audioCallback);
        
        // Prepare processor
        auto* device = deviceManager->getCurrentAudioDevice();
        if (device) {
            processor->prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
            std::cout << "🎵 Audio: " << device->getCurrentSampleRate() << "Hz" << std::endl;
        }
        
        // Create simple GUI
        editor = std::unique_ptr<juce::AudioProcessorEditor>(processor->createEditor());
        if (editor) {
            mainWindow = std::make_unique<MainWindow>("FootstepDetector - MIDI FREE", editor.get());
        }
        
        std::cout << "✅ FootstepDetector ready - NO MIDI ERRORS!" << std::endl;
    }

    void shutdown() override
    {
        if (deviceManager) {
            deviceManager->removeAudioCallback(&audioCallback);
            deviceManager->closeAudioDevice();
        }
        editor.reset();
        mainWindow.reset();
        processor.reset();
        deviceManager.reset();
    }

    void systemRequestedQuit() override { quit(); }

private:
    class AudioCallback : public juce::AudioIODeviceCallback
    {
    public:
        AudioCallback(FootstepDetectorAudioProcessor* p) : processor(p) {}
        
        void audioDeviceIOCallback(const float** inputChannelData,
                                  int numInputChannels,
                                  float** outputChannelData,
                                  int numOutputChannels,
                                  int numSamples) override
        {
            if (processor) {
                juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
                
                // Copy input to buffer
                for (int ch = 0; ch < juce::jmin(numInputChannels, numOutputChannels); ++ch) {
                    if (inputChannelData[ch]) {
                        buffer.copyFrom(ch, 0, inputChannelData[ch], numSamples);
                    }
                }
                
                juce::MidiBuffer midiBuffer; // Empty - no MIDI
                processor->processBlock(buffer, midiBuffer);
            }
        }
        
        void audioDeviceAboutToStart(juce::AudioIODevice* device) override
        {
            if (processor) {
                processor->prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
            }
        }
        
        void audioDeviceStopped() override
        {
            if (processor) {
                processor->releaseResources();
            }
        }
        
    private:
        FootstepDetectorAudioProcessor* processor;
    } audioCallback{processor.get()};

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, juce::Component* content)
            : DocumentWindow(name, juce::Colours::darkgrey, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(content, true);
            setResizable(true, true);
            centreWithSize(400, 300);
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
    std::unique_ptr<MainWindow> mainWindow;
};

// NUCLEAR: Completely replace JUCE's MIDI-enabled application
START_JUCE_APPLICATION(MIDIFreeStandaloneApp)
