//
//  DspHelpers.hpp
//  DspHelpers
//
//  Created by Joshua Hodge on 25/07/2020.
//  Copyright © 2020 The Audio Programmer. All rights reserved.
//

#ifndef DspHelpers_hpp
#define DspHelpers_hpp

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <queue>
#include <numeric>
#include <cassert>

// A simple collection of helpful DSP algorithms with no dependencies.  Many of these algorithms were 

namespace tap
{

template <typename Type>
class Decibels
{
    /** Convert raw gain (between 0 - 1) to dBFS */
    static Type convertGainToDecibels (Type rawGain)
    {
        return 20 * std::log10 (rawGain);
    }
    
    /** Convert dBFS to raw gain (between 0 - 1) */
    static Type convertDecibelsToGain (Type decibels)
    {
        return std::pow (Type (10), decibels / 20);
    }
};

// =================================================================

template <typename Type>
class Amplitude
{
public:
    /** Find the maximum peak of a signal */
    void updatePeakSignal (Type sample) noexcept
    {
        peakVal = std::max (std::abs (sample), peakVal);
    }
    
    /** Return the max peak */
    Type getPeak() const noexcept
    {
        return peakVal;
    }
    
    /** Reset the peak */
    void reset() noexcept
    {
        peakVal = 0;
    }
    
    /**  Find the root mean square of a signal.  The max window size is 192000, which is one second of audio at 192 kHz.
         std::pow was deliberately avoided because multiplying directly is more efficient.
     */
    void updateRms (Type sample, const int windowSize) noexcept
    {
        // Your windowSize is too big!
        assert (windowSize < maxWindowSize);
        
        auto oldSignal = rmsWindow[index];
        rmsWindow[index] = sample;
        
        sum += (sample * sample) - (oldSignal * oldSignal);
        
        rmsVal = std::sqrt (sum / windowSize);
        
        index++;
        
        if (index >= windowSize)
        {
            index = 0;
            sum = 0;
            
            // Recalculate to avoid floating point error drift
            for (auto i = 0; i < windowSize; ++i)
                sum += rmsWindow[i] * rmsWindow[i];
        }
    }
    
    /** Return the rms of a signal */
    Type getRms() const noexcept
    {
        return rmsVal;
    }
    
private:
    Type peakVal;
    Type rmsVal;
    
    // One second of audio at 192kHz
    static constexpr int maxWindowSize = 192000;
    Type rmsWindow [maxWindowSize];
    
    int index = 0;
    int sum = 0;
};

// =================================================================

template <typename Type>
class SynthWave
{
public:
    /** Pass the sample rate to the DSP algorithm*/
    void prepareToPlay (double& sampleRate) noexcept
    {
        currentSampleRate = sampleRate;
        timeStep = 1 / currentSampleRate;
    }
    
    /**  Generate a sine wave with the equation (2 * pi * frequency * time + phaseOffset).
         This isn't the most efficient way to generate a sine wave, but it's a good learning
         tool for beginners who are generating sound and learning DSP for the first time.
     */
    Type processSine (const Type& frequency, const int phaseOffset = 0)
    {
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // Reset time once we complete a cycle
        if (currentTime >= 1.0)
            currentTime = 0.0;
        
        auto sample = std::sin (2.0 * pi * frequency * currentTime + phaseOffset);
        
        // Need to increment time for the next time this function calls
        currentTime += timeStep;
        
        return sample;
    }
    
    /**  Generate an additive square wave by summing odd harmonics
         of sine waves from the fundamental frequency to the Nyquist.
         Based on square wave additive synthesis equation in Hack Audio by Eric Tarr.
     */
    Type processSquare (const Type& frequency, const int phaseOffset = 0)
    {
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // Reset time once we complete a cycle
        if (currentTime >= 1.0)
            currentTime = 0.0;
        
        auto sample = 2.0f * pi * frequency * currentTime + phaseOffset;
        
        // Find the max harmonic frequency
        auto maxHarmonic = std::floor (currentSampleRate / (2.0f * frequency));
        
        Type sumOfSines = 0.0;
        
        // Add sine waves together
        for (auto harmonic = 1.0; harmonic <= maxHarmonic; harmonic += 2.0)
        {
            sumOfSines +=  1.0 / harmonic * std::sin (harmonic * sample);
        }
                
        auto output = 4 / pi * sumOfSines;
        
        // Need to increment time for the next time this function calls
        currentTime += timeStep;
        
        return output;
    }
    
    /**  Generate an additive square wave by summing even harmonics
         of sine waves from the fundamental frequency to the Nyquist.
         Based on saw wave additive synthesis equation in Hack Audio by Eric Tarr.
     */
    Type processSaw (const Type& frequency, const int phaseOffset = 0)
    {
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // Reset time once we complete a cycle
        if (currentTime >= 1.0)
            currentTime = 0.0;
        
        auto sample = 2.0f * pi * frequency * currentTime + phaseOffset;
        
        // Find the max harmonic frequency
        auto maxHarmonic = std::floor (currentSampleRate / (2.0f * frequency));
        
        Type sumOfSines = 0.0;
        
        // Add sine waves together
        for (auto harmonic = 1.0; harmonic <= maxHarmonic; ++harmonic)
        {
            sumOfSines +=  1.0 / harmonic * std::sin (harmonic * sample);
        }
                
        auto output = (1 / 2) - (1 / pi) * sumOfSines;
        
        // Need to increment time for the next time this function calls
        currentTime += timeStep;
        
        return output;
    }
    
    /**  Generate an additive Triangle wave by summing odd harmonics
         of sine waves from the fundamental frequency to the Nyquist.
         Based on triangle wave additive synthesis equation in Hack Audio by Eric Tarr.
     */
    Type processTriangle (const Type& frequency, const int phaseOffset = 0)
    {
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // Reset time once we complete a cycle
        if (currentTime >= 1.0)
            currentTime = 0.0;
        
        auto sample = 2.0f * pi * frequency * currentTime + phaseOffset;
        
        // Find the max harmonic frequency
        auto maxHarmonic = std::floor (currentSampleRate / (2.0f * frequency));
        
        Type sumOfSines = 0.0;
        
        // Add sine waves together
        for (auto harmonic = 1.0; harmonic <= maxHarmonic; harmonic += 2.0)
        {
            sumOfSines +=  (1.0 / (harmonic * harmonic)) * std::sin (harmonic * sample);
        }
                
        auto output = (8 / (pi * pi)) * sumOfSines;
        
        // Need to increment time for the next time this function calls
        currentTime += timeStep;
        
        return output;
    }
    
    /**  Generate an additive impulse train by summing harmonics
         of sine waves from the fundamental frequency to the Nyquist.
         Based on impulse train additive synthesis equation in Hack Audio by Eric Tarr.
     */
    Type processImpulseTrain (const Type& frequency, const int phaseOffset = 0)
    {
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // Reset time once we complete a cycle
        if (currentTime >= 1.0)
            currentTime = 0.0;
        
        auto sample = 2.0f * pi * frequency * currentTime + phaseOffset;
        
        // Find the max harmonic frequency
        auto maxHarmonic = std::floor (currentSampleRate / (2.0f * frequency));
        
        Type sumOfSines = 0.0;
        
        // Add sine waves together
        for (auto harmonic = 1.0; harmonic <= maxHarmonic; ++harmonic)
        {
            sumOfSines += std::sin (harmonic * sample);
        }
                
        auto output = (pi / (2 * maxHarmonic)) * sumOfSines;
        
        // Need to increment time for the next time this function calls
        currentTime += timeStep;
        
        return output;
    }
    
private:
    static constexpr Type pi = 3.141592653589793238;
    double currentSampleRate = 0;
    Type currentTime = 0;
    Type timeStep = 0;
};

// =================================================================

/** Allows selection of the tremolo wave type in the tremolo class*/
enum class TremoloWaveType
{
    Sine,
    Saw,
    Square,
    Triangle
};

// =================================================================

template <typename Type>
class Tremolo
{
public:
    /** Pass the sample rate to the DSP algorithm*/
    void prepareToPlay (double& sampleRate) noexcept
    {
        currentSampleRate = sampleRate;
        modulator.prepareToPlay (sampleRate);
    }
    
    void setFrequency (Type freq) noexcept
    {
        frequency = freq;
    }
    
    void setWaveType (const TremoloWaveType& type) noexcept
    {
        waveType = type;
    }
    
    Type process (Type& sample, float amp)
    {
        // Careful!  Your tremolo amp should be between 0.0 and 1.0
        assert (amp >= 0.0f && amp <= 1.0f);
        
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // You need to set the frequency of your modulator frequency using setFrequency()
        assert (frequency > 0);
        
        return sample * (amp * getModulator());
    }
    
private:
    SynthWave<Type> modulator;
    TremoloWaveType waveType = TremoloWaveType::Sine;
    double currentSampleRate = 0;
    Type frequency = 0;
    
    Type getModulator()
    {
        switch (waveType)
        {
            case (TremoloWaveType::Sine):
                return std::abs (modulator.processSine (frequency));
            case (TremoloWaveType::Saw):
                return std::abs (modulator.processSaw (frequency));
            case (TremoloWaveType::Square):
                return std::abs (modulator.processSquare (frequency));
            case (TremoloWaveType::Triangle):
                return std::abs (modulator.processTriangle (frequency));
            default:
                return std::abs (modulator.processSine (frequency));
        }
    }
};

// =================================================================

enum class FadeType
{
    In,
    Out
};

template <typename Type>
class AmplitudeFade
{
public:
    /** Build a ramp between 0 and 1 to use for fade ins or fade outs.  A curve of 1 will give a linear curve, less than 1 makes the curve more exponential,
        and more than 1 makes the curve more logarithmic.  Curve function courtesy of Pelle in the TAP Discord.
     */
    
    void buildRamp (const int numSamplesToFade, const FadeType& fadeInOrOut, float curve) noexcept
    {
        // Prevent division by 0
        if (curve == 0.0f)
            curve = 0.1f;
        
        auto start = fadeType == FadeType::Out ? 1.0f : 0.0f;
        auto end   = fadeType == FadeType::Out ? 0.0f : 1.0f;
        
        // Linear fade
        for (int i = 0; i < numSamplesToFade; ++i)
        {
            auto x = start + (end - start) * (i / numSamplesToFade);
            fadeRamp[i] = (std::expf (curve * x) - 1) / (std::expf (curve) - 1);
        }
    }
    
private:
    // Array to hold ramp values (max buffer size of 8196)
    static constexpr int rampSize = 8192;
    Type fadeRamp [rampSize] = 0;
    FadeType fadeType = FadeType::In;
    
};

// =================================================================

enum class PanningType
{
    Linear,             // Equal amplitude panning
    PowerSineLaw,       // Equal power panning using sine law
    PowerSquareLaw,     // Equal power panning using square law
    ModifiedSineLaw,    // Offers benefits of amplitude and power using modified sine law
    ModifiedSquareLaw   // Offers benefits of amplitude and power using modified square law
};

template <typename Type>
class Panner
{
public:
    void setPanningType (const PanningType& type) noexcept
    {
        panningType = type;
    }
    
    Type process (const int& channel, Type& sample, const Type& panValue, const int& numChannels)
    {
        /* Expects a panning parameter with a range of -0.0 to 1.0 where
           0.5 is center, -0.0 is only the left speaker, and 1.0 is the right speaker */
        assert (panValue >= 0.0 && panValue <= 1.0);
        
        // Only works for a stereo signal
        assert (numChannels == 2);
        
        auto value = channel == 0 ? 1.0 - panValue : panValue;
        
        switch (panningType)
        {
            case PanningType::Linear:
                return sample * (value);
                break;
            case PanningType::PowerSineLaw:
                return sample * (std::sin ((value) * pi / 2.0));
                break;
            case PanningType::PowerSquareLaw:
                return sample * (std::sqrt (value));
                break;
            case PanningType::ModifiedSineLaw:
                return sample * (std::pow (value, 0.75));
                break;
            case PanningType::ModifiedSquareLaw:
                return sample * (std::sqrt ((value) * std::sin ((value) * pi / 2.0)));
                break;
        }
    }
    
private:
    static constexpr Type pi = 3.141592653589793238;
    PanningType panningType = PanningType::Linear;
};

// =================================================================

/**
    A class for encoding a stereo signal into mid / side components for further processing,
    or for decoding a mid / side signal into a stereo signal
 */

template <typename Type>
class MidSideProcessing
{
public:
    /** Encodes a stereo signal into mid and side signals */
    Type encode (const int& channel, Type& leftSample, Type& rightSample)
    {
        // Only works on stereo signal
        assert (channel >= 0 && channel <= 1);
        
        return channel == 0 ? 0.5 * leftSample - rightSample
                            : 0.5 * leftSample + rightSample;
    }
    
    /** Decodes a mid and side signal into a stereo signal */
    Type decode (const int& channel, Type& middleSample, Type& sideSample)
    {
        // Only works on stereo signal
        assert (channel >= 0 && channel <= 1);
        
        return channel == 0 ? middleSample + sideSample
                            : middleSample - sideSample;
    }
    
    /**
        Encodes a stereo signal into mid and side signals while narrowing or widening the stereo field.
        A factor less than 1 will return a narrowed signal, while a factor more than 1 will return a widened signal
     */
    
    Type stereoFieldNarrowOrWiden (const int& channel, Type& leftSample, Type& rightSample, Type& factor)
    {
        // Only works on stereo signal
        assert (channel >= 0 && channel <= 1);
        
        // channel == 0 returns the side signal, channel == 1 returns the mid signal.
        return channel == 0 ? factor * (leftSample - rightSample)
                            : (2 - factor) * (leftSample + rightSample);
    }
};

/**
    A class for converting a stereo signal to polar coordinates, then to Cartesian coordinates for the purpose
    of creating a goniometer to visualize a stereo signal.
 */

template <typename Type>
class Goniometer
{
public:
    /** Calculate polar coordinates from a stereo sample */
    std::tuple<Type, Type> calculatePolarCoordinates (const Type& leftSample, const Type& rightSample)
    {
        auto theta  = std::atan2 (leftSample, rightSample);
        auto radius = std::sqrt ((leftSample * leftSample) + (rightSample * rightSample));
        return std::make_tuple (theta, radius);
    }
    
    /** Convert polar coordinates to Cartesian coordinates */
    std::tuple<Type, Type> calculateCartesianCoordinates (std::tuple<Type, Type>& radiusAndTheta)
    {
        auto radius = std::get<0>(radiusAndTheta);
        auto theta  = std::get<1>(radiusAndTheta);
        
        auto x = radius * std::cos (theta);
        auto y = radius * std::sin (theta);
        
        return std::make_tuple (x, y);
    }
    
private:
    static constexpr Type pi = 3.141592653589793238;
};

template <typename Type>
class Distortion
{
public:
    Type processInfiniteClipping (Type& sample)
    {
        if (sample == 0.0)
            return 0.0;
        else
            return sample >= 0.0 ? 1.0 : -1.0;
    }
    
    Type processHalfWaveRectification (Type& sample)
    {
        return sample < 0.0 ? 0.0 : sample;
    }
    
    Type processFullWaveRectification (Type& sample)
    {
        return sample < 0.0 ? std::abs (sample) : sample;
    }
    
    Type processHardClipping (Type& sample, const Type& maxThresh)
    {
        //Values should be between 0.0 and 1.0
        assert (maxThresh >= 0.0 && maxThresh <= 1.0);
        
        if (sample >= maxThresh)
            return maxThresh;
        else if (sample <= -maxThresh)
            return -maxThresh;
        else
            return sample;
    }
    
    Type processCubic (Type& sample)
    {
        return sample - 1 / 3 * (sample * sample * sample);
    }
    
    Type processArcTan (Type& sample, const Type& coefficient)
    {
        // The coefficient should be between 1.0 and 10.0
        assert (coefficient >= 1.0 && coefficient <= 10.0);
        return 2 / pi * std::atan (coefficient * sample);
    }
    
private:
    static constexpr Type pi = 3.141592653589793238;
};

} // namespace tap

#endif /* DspHelpers_hpp */
