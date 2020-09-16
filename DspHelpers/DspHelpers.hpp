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
    
    /** Find the root mean square of a signal.  The max window size is 192000, which is one second of audio at 192 kHz.
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

} // namespace tap

#endif /* DspHelpers_hpp */
