#include "FourierTransformer.hpp"
#include "WaveFile.hpp"

int main()
{
    WaveFile file = WaveFile("../samples/8-bit.wav");
    FourierTransformer transformer;
    transformer.fft(file.samples[0], file.numSamples);
    return 0;
}