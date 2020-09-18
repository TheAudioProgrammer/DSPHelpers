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
class SineWave
{
public:
    /** Pass the sample rate to the DSP algorithm*/
    void prepareToPlay (double& sampleRate)
    {
        currentSampleRate = sampleRate;
        timeStep = 1 / currentSampleRate;
    }
    
    /**  Generate a sine wave with the equation (2 * pi * frequency * time + phaseOffset).
         This isn't the most efficient way to generate a sine wave, but it's a good learning
         tool for beginners who are generating sound and learning DSP for the first time.
     */
    Type calculate (const Type& frequency, const int phaseOffset = 0)
    {
        // Ensure our frequency is in the range of human hearing
        assert (frequency >= 20 && frequency <= 20000);
        
        // You must set your sample rate in prepareToPlay
        assert (currentSampleRate > 0);
        
        // Make sure we're not running off the edge of our time max
        if (currentTime >= std::numeric_limits<float>::max())
            currentTime = 0.0;
        
        auto sample = std::sin (2.0 * pi * frequency * currentTime + phaseOffset);
        
        // Need to increment time for the next time this function calls
        currentTime += timeStep;
        
        return sample;
    }
    
private:
    static constexpr Type pi = 3.141592653589793238;
    double currentSampleRate = 0;
    Type currentTime = 0;
    Type currentAngle = 0;
    Type timeStep = 0;
};

} // namespace tap

#endif /* DspHelpers_hpp */
