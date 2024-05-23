#include "WaveFile.hpp"

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
    // TODO: validate "RIFF", "WAVE", "fmt " and "data"
    headerPointer += 0; // 0:  ChunkID
    headerPointer += 4; // 4:  ChunkSize
    headerPointer += 4; // 8:  Format

    headerPointer += 4; // 12: Subchunk1ID
    headerPointer += 4; // 16: Subchunk1Size
    headerPointer += 4; // 20: AudioFormat
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
    samples = std::vector<uint32_t>(numSamples);
    for (int i = 0; i < numSamples; i++)
    {
        byteStream.read(sampleBuffer, bytesPerSample);
        samples[i] = littleEndian(sampleBuffer, bytesPerSample);
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