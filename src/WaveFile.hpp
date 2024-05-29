#ifndef WAVE_HEADER
#define WAVE_HEADER

#include <iostream>
#include <vector>

class WaveFile
{
public:
    static const std::size_t HEADER_SIZE = 44;

    uint32_t numSamples;
    uint32_t numChannels;
    uint32_t sampleRate;
    std::vector<std::vector<double>> samples;

    WaveFile(std::string filename);
    void write(std::string filename);

private:
    // uint32_t since header data can contain up to 4 bytes = 32 bits
    uint32_t bitsPerSample;

    uint32_t littleEndianToInt(char* bytes, int size);
    uint32_t bigEndianToInt(char* bytes, int size);
    std::string intToLittleEndian(int value, int size);
};

#endif