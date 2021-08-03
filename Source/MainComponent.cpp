#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    slider.setSliderStyle (juce::Slider::SliderStyle::LinearHorizontal);
    slider.setRange (0.0f, 1.0f);
    slider.setValue (0.5f);
    slider.onValueChange = [&]() { sliderValue.store (slider.getValue()); };
    addAndMakeVisible (slider);
    
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
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    for (auto i = 0; i < outputs; ++i)
    {
        synthWave1[i].prepareToPlay (sampleRate);
        synthWave2[i].prepareToPlay (sampleRate);
        tremolo[i].prepareToPlay (sampleRate);
    }
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    auto numSamples = bufferToFill.buffer->getNumSamples();
    
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
    {
        // I intentionally made this stereo only for simplicity
        jassert (bufferToFill.buffer->getNumChannels() == outputs);
        
        auto* buffer = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
    
        for (auto sample = 0; sample < numSamples; ++sample)
        {
            buffer[sample] = synthWave1[channel].processSine (200.0f);
            buffer[sample] = distortion[channel].processBitCrush (buffer[sample], 4.0f);
            buffer[sample] *= 0.125f;
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
    slider.setBounds (getLocalBounds().reduced (30));
}




//tremolo[channel].setFrequency (5.0f);
//tremolo[channel].setWaveType (tap::TremoloWaveType::Sine);
//panner.setPanningType (tap::PanningType::PowerSquareLaw);
//buffer[sample] =  tremolo[channel].process (buffer[sample], 0.5f);

//            meter.updateRms (buffer[sample], numSamples);
//            meter.updatePeakSignal (buffer[sample]);

// buffer[sample] = panner.process (channel, buffer[sample], sliderValue.load(), bufferToFill.buffer->getNumChannels());

//buffer[sample] = distortion[channel].processArcTan (buffer[sample], 10.0);
//buffer[sample] = distortion[channel].processExponentialSoftClipping (sample, 1.0);
//buffer[sample] = distortion[channel].processHardClipping (buffer[sample], 0.01);
//buffer[sample] = distortion[channel].processCubic (sample);
