#include "WaveFile.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

WaveFile::WaveFile(std::string filename)
{
    // WAVE file format: http://soundfile.sapp.org/doc/WaveFormat/
    std::ifstream byteStream(filename, std::ios::binary);
    char headerBuffer[HEADER_SIZE];
    byteStream.read(headerBuffer, HEADER_SIZE);

    char* headerPointer = headerBuffer;
    headerPointer += 0; // 0:  ChunkID
    assert(bigEndian(headerPointer, 4) == 0x52494646); // RIFF
    headerPointer += 4; // 4:  ChunkSize
    headerPointer += 4; // 8:  Format
    assert(bigEndian(headerPointer, 4) == 0x57415645); // WAVE

    headerPointer += 4; // 12: Subchunk1ID
    assert(bigEndian(headerPointer, 4) == 0x666d7420); // fmt
    headerPointer += 4; // 16: Subchunk1Size
    headerPointer += 4; // 20: AudioFormat
    uint32_t audioFormat = littleEndian(headerPointer, 2);
    bool pcm = audioFormat == 1;
    headerPointer += 2; // 22: NumChannels
    numChannels = littleEndian(headerPointer, 2);
    headerPointer += 2; // 24: SampleRate
    sampleRate = littleEndian(headerPointer, 4);
    headerPointer += 4; // 28: ByteRate
    headerPointer += 4; // 32: BlockAlign
    headerPointer += 2; // 34: BitsPerSample
    bitsPerSample = littleEndian(headerPointer, 2);
    size_t bytesPerSample = bitsPerSample / 8;

    headerPointer += 2; // 36: Subchunk2ID
    assert(bigEndian(headerPointer, 4) == 0x64617461); // data
    headerPointer += 4; // 40: Subchunk2Size
    uint32_t dataSize = littleEndian(headerPointer, 4);
    numSamples = dataSize / numChannels / bytesPerSample;

    // sanity check
    std::cout << "numChannels: " << numChannels << "\n";
    std::cout << "sampleRate: " << sampleRate << "\n";
    std::cout << "bitsPerSample: " << bitsPerSample << "\n";
    std::cout << "numSamples: " << numSamples << "\n";

    // TODO: write a test to check this works as expected
    char* sampleBuffer = (char*) malloc(bytesPerSample);
    samples = std::vector<Channel>(numChannels, Channel(numSamples));
    for (int i = 0; i < numSamples; i++)
    {
        for (int j = 0; j < numChannels; j++)
        {
            byteStream.read(sampleBuffer, bytesPerSample);
            samples[j][i] = littleEndian(sampleBuffer, bytesPerSample);

            // normalize data to [-1.0, 1.0) 
            if (pcm) {
                if (bitsPerSample == 8) {
                    // values in the range [0, 255]
                    samples[j][i] = samples[j][i] / 255.0 * 2.0 - 1.0;
                } else if (bitsPerSample == 16) {
                    // values in the range [-32768, 32767]
                    samples[j][i] = samples[j][i] / 32678.0;
                }
            }
        }
    }
}

uint32_t WaveFile::littleEndian(char* bytes, size_t size)
{
    // least significant byte at smallest address
    uint32_t value = 0;
    for (int i = 0; i < size; i++)
    {
        // bitmask is necessary to prevent sign extension
        uint32_t offset = i * 8;
        uint32_t mask = 255 << offset;
        value |= (bytes[i] << offset) & mask;
    }
    return value;
}

uint32_t WaveFile::bigEndian(char* bytes, size_t size)
{
    // mosst significant byte at biggest address
    uint32_t value = 0;
    for (int i = 0; i < size; i++)
    {
        // bitmask is necessary to prevent sign extension
        uint32_t offset = (size - i - 1) * 8;
        uint32_t mask = 255 << offset;
        value |= bytes[i] << offset;
    }
    return value;
}