#include "FourierTransformer.hpp"
#include "WaveFile.hpp"

#include <fstream>

int main()
{
    WaveFile file = WaveFile("../samples/16-bit.wav");
    FourierTransformer transformer;

    for (int i = 0; i < file.samples.size(); i++)
    {
        // FFT -> inverse FFT
        Channel input = file.samples[i];
        auto transformed = transformer.fft(input, file.numSamples);
        Channel transformedChannel = Channel(transformed.size());
        int numSamples = transformed.size();
        auto inverted = transformer.ifft(transformed, numSamples);

        // write back
        for (int j = 0; j < file.numSamples; j++)
        {
            file.samples[i][j] = inverted[j].real();
        }
    }

    file.write("../demo/16-bit-copy.wav");

    return 0;
}