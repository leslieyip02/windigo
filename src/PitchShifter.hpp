#ifndef PITCHSHIFTER_HEADER
#define PITCHSHIFTER_HEADER

#include "FourierTransformer.hpp"
#include "WaveFile.hpp"

#include <vector>

class PitchShifter
{
public:
    void shift(WaveFile& file, int steps);
    PitchShifter(int frameSize, int overlapFactor);

private:
    int frameSize;
    int overlapFactor;
    FourierTransformer transformer;

    std::vector<double> hanningWindow(int frameSize);
};

#endif