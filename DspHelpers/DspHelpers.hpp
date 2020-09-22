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

// A simple collection of helpful DSP algorithms with no dependencies.  

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

enum class TremoloWaveType
{
    Sine,
    Saw,
    Square,
    Triangle
};

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
    
    Type processTremolo (Type& sample, float amp)
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

} // namespace tap

#endif /* DspHelpers_hpp */
