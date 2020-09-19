#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
//    for (auto i = 0; i < outputs; ++i)
//    {
//        sineWave[i].prepareToPlay (sampleRate);
//    }
    
    for (auto i = 0; i < outputs; ++i)
    {
        synthWave[i].prepareToPlay (sampleRate);
    }
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    auto numSamples = bufferToFill.buffer->getNumSamples();
    auto amp = 0.125f;
    auto freq = 150.0f;
    
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        // I intentionally made this stereo only for simplicity
        jassert (bufferToFill.buffer->getNumChannels() == outputs);
        
        auto* buffer  = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
    
        for (auto sample = 0; sample < numSamples; ++sample)
        {
            buffer[sample] = amp * synthWave[channel].processSaw (freq);
            meter.updateRms (buffer[sample], numSamples);
            meter.updatePeakSignal (buffer[sample]);
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
