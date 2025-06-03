#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"

class FootstepDetectorApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "FootstepDetector"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    
    void initialise(const juce::String&) override
    {
        processor = std::make_unique<FootstepDetectorAudioProcessor>();
        
        // Initialize audio device manager WITHOUT MIDI
        audioDeviceManager.initialiseWithDefaultDevices(2, 2);
        audioDeviceManager.addAudioCallback(&audioCallback);
        
        // Create main window
        mainWindow = std::make_unique<MainWindow>("FootstepDetector", processor.get());
    }
    
    void shutdown() override
    {
        audioDeviceManager.removeAudioCallback(&audioCallback);
        mainWindow.reset();
        processor.reset();
    }
    
private:
    std::unique_ptr<FootstepDetectorAudioProcessor> processor;
    juce::AudioDeviceManager audioDeviceManager;
    
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
            if (processor)
            {
                juce::AudioBuffer<float> buffer(outputChannelData, numOutputChannels, numSamples);
                juce::MidiBuffer midiBuffer;
                processor->processBlock(buffer, midiBuffer);
            }
        }
        
        void audioDeviceAboutToStart(juce::AudioIODevice* device) override
        {
            if (processor)
                processor->prepareToPlay(device->getCurrentSampleRate(), device->getCurrentBufferSizeSamples());
        }
        
        void audioDeviceStopped() override
        {
            if (processor)
                processor->releaseResources();
        }
        
    private:
        FootstepDetectorAudioProcessor* processor;
    } audioCallback{processor.get()};
    
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String& name, FootstepDetectorAudioProcessor* p)
            : DocumentWindow(name, juce::Colours::lightgrey, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new FootstepDetectorAudioProcessorEditor(*p), true);
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }
        
        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };
    
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(FootstepDetectorApplication)