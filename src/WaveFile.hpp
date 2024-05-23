#include <iostream>
#include <vector>

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

    // uint32_t to fit up to 32-bit samples
    std::vector<uint32_t> samples;

    uint32_t littleEndian(char* bytes, size_t size);
    uint32_t bigEndian(char* bytes, size_t size);
};