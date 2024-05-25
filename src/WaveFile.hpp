#include <iostream>
#include <vector>

#define Channel std::vector<double>

class WaveFile
{
public:
    static const size_t HEADER_SIZE = 44;

    WaveFile(std::string filename);

private:
    // uint32_t since header data can contain up to 4 bytes = 32 bits
    uint32_t numChannels;
    uint32_t sampleRate;
    uint32_t bitsPerSample;
    uint32_t numSamples;

    std::vector<Channel> samples;

    uint32_t littleEndian(char* bytes, size_t size);
    uint32_t bigEndian(char* bytes, size_t size);
};