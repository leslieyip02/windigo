#ifndef WAVE_HEADER
#define WAVE_HEADER

#include <iostream>
#include <vector>

#define Channel std::vector<double>

class WaveFile
{
public:
    static const std::size_t HEADER_SIZE = 44;

    uint32_t numSamples;
    std::vector<Channel> samples;

    WaveFile(std::string filename);
    void write(std::string filename);

private:
    // uint32_t since header data can contain up to 4 bytes = 32 bits
    uint32_t numChannels;
    uint32_t sampleRate;
    uint32_t bitsPerSample;

    uint32_t littleEndianToInt(char* bytes, int size);
    uint32_t bigEndianToInt(char* bytes, int size);

    std::string intToLittleEndian(int value, int size);
};

#endif