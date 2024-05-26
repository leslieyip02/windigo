#include "FourierTransformer.hpp"
#include "WaveFile.hpp"

#include <fstream>

int main(int argc, char** argv)
{
    std::string inputFilename = argc >= 2 ? argv[1] : "../samples/8-bit.wav";
    std::string outputFilename = argc >= 3 ? argv[2] : "output.wav";

    WaveFile file = WaveFile(inputFilename);
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

    file.write(outputFilename);
    return 0;
}