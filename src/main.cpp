#include "FourierTransformer.hpp"
#include "WaveFile.hpp"

#include <fstream>

int main()
{
    WaveFile file = WaveFile("../samples/8-bit.wav");
    FourierTransformer transformer;

    Channel input = file.samples[0];
    auto transformed = transformer.fft(input, file.numSamples);
    Channel transformedChannel = Channel(transformed.size());
    int numSamples = transformed.size();
    auto inverted = transformer.ifft(transformed, numSamples);

    // TODO: write output into a .wav file
    // output to csv (for now)
    std::ofstream csv("../demo/fft.csv");
    csv << "input,fft(real),fft(imaginary),ifft(real),ifft(imaginary)\n";
    for (int i = 0; i < numSamples; i++)
    {
        csv << (i >= file.numSamples ? 0 : input[i]) << ",";
        csv << transformed[i].real() << "," << transformed[i].imag() << ",";
        csv << inverted[i].real() << "," << inverted[i].imag() << "\n";
    }

    return 0;
}