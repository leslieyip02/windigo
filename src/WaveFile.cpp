#include "WaveFile.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

WaveFile::WaveFile(std::string filename)
{
    // std::ios_base::binary is necessary for windows
    std::ifstream byteStream(filename, std::ios::binary);
    char headerBuffer[HEADER_SIZE];
    byteStream.read(headerBuffer, HEADER_SIZE);

    // WAVE file format: http://soundfile.sapp.org/doc/WaveFormat/
    char* headerPointer = headerBuffer;
    headerPointer += 0; // 0:  ChunkID
    assert(bigEndianToInt(headerPointer, 4) == 0x52494646); // RIFF
    headerPointer += 4; // 4:  ChunkSize
    headerPointer += 4; // 8:  Format
    assert(bigEndianToInt(headerPointer, 4) == 0x57415645); // WAVE

    headerPointer += 4; // 12: Subchunk1ID
    assert(bigEndianToInt(headerPointer, 4) == 0x666d7420); // fmt
    headerPointer += 4; // 16: Subchunk1Size
    headerPointer += 4; // 20: AudioFormat
    uint32_t audioFormat = littleEndianToInt(headerPointer, 2);
    headerPointer += 2; // 22: NumChannels
    numChannels = littleEndianToInt(headerPointer, 2);
    headerPointer += 2; // 24: SampleRate
    sampleRate = littleEndianToInt(headerPointer, 4);
    headerPointer += 4; // 28: ByteRate
    headerPointer += 4; // 32: BlockAlign
    headerPointer += 2; // 34: BitsPerSample
    bitsPerSample = littleEndianToInt(headerPointer, 2);
    int bytesPerSample = bitsPerSample / 8;

    headerPointer += 2; // 36: Subchunk2ID
    assert(bigEndianToInt(headerPointer, 4) == 0x64617461); // data
    headerPointer += 4; // 40: Subchunk2Size
    uint32_t dataSize = littleEndianToInt(headerPointer, 4);
    numSamples = dataSize / numChannels / bytesPerSample;

    bool pcm = audioFormat == 1;
    if (pcm)
    {
        // only 8-bit and 16-bit supported
        assert(bitsPerSample == 8 || bitsPerSample == 16);
    }

    // TODO: add unit test
    char* sampleBuffer = (char*) malloc(bytesPerSample);
    samples = std::vector<std::vector<double>>(numChannels, std::vector<double>(numSamples));
    for (int i = 0; i < numSamples; i++)
    {
        for (int j = 0; j < numChannels; j++)
        {
            byteStream.read(sampleBuffer, bytesPerSample);
            uint32_t rawValue = littleEndianToInt(sampleBuffer, bytesPerSample);

            // normalize data to [-1.0, 1.0) 
            if (pcm)
            {
                if (bitsPerSample == 8)
                {
                    // values in the range [0, 255]
                    samples[j][i] = (double) rawValue / 255.0 * 2.0 - 1.0;
                }
                else if (bitsPerSample == 16)
                {
                    // values in the range [-32768, 32767]
                    samples[j][i] = (double) static_cast<int16_t>(rawValue) / 32767.0;
                }
            }
            else
            {
                // if IEEE, the float should already be [-1.0, 1,0)
                samples[j][i] = static_cast<double>(rawValue);
            }
        }
    }
}

void WaveFile::write(std::string filename)
{
    // std::ios_base::binary is necessary for windows
    std::ofstream output(filename, std::ios_base::binary);

    int bytesPerSample = bitsPerSample / 8;
    int subchunk1Size = 16; // 16 for PCM
    int audioFormat = 1; // PCM by default
    int subchunk2Size = numSamples * numChannels * bytesPerSample;
    int chunkSize = 4 + (8 + subchunk1Size) + (8 + subchunk2Size);

    int blockAlign = numChannels * bytesPerSample;
    int byteRate = sampleRate * blockAlign;

    output << "RIFF"; // 0:  ChunkID
    output << intToLittleEndian(chunkSize, 4); // 4:  ChunkSize
    output << "WAVE"; // 8:  Format

    output << "fmt "; // 12: Subchunk1ID
    output << intToLittleEndian(subchunk1Size, 4); // 16: Subchunk1Size
    output << intToLittleEndian(audioFormat, 2); // 20: AudioFormat
    output << intToLittleEndian(numChannels, 2); // 22: NumChannels
    output << intToLittleEndian(sampleRate, 4); // 24: SampleRate
    output << intToLittleEndian(byteRate, 4); // 28: ByteRate
    output << intToLittleEndian(blockAlign, 2); // 32: BlockAlign
    output << intToLittleEndian(bitsPerSample, 2); // 34: BitsPerSample

    output << "data"; // 36: Subchunk2ID
    output << intToLittleEndian(subchunk2Size, 4); // 40: Subchunk2Size

    for (int i = 0; i < numSamples; i++)
    {
        for (int j = 0; j < numChannels; j++)
        {
            // unnormalize
            if (bitsPerSample == 8)
            {
                // values in the range [0, 255]
                samples[j][i] = (samples[j][i] + 1.0) / 2.0 * 255.0;
            }
            else if (bitsPerSample == 16)
            {
                // values in the range [-32768, 32767]
                samples[j][i] = samples[j][i] * 32767.0;
            }

            output << intToLittleEndian(samples[j][i], bytesPerSample);
        }
    }
}

uint32_t WaveFile::littleEndianToInt(char* bytes, int size)
{
    // least significant byte at smallest address
    uint32_t value = 0;
    for (int i = 0; i < size; i++)
    {
        // bitmask is necessary to prevent sign extension
        uint32_t offset = i * 8;
        uint32_t mask = 0xff << offset;
        value |= (bytes[i] << offset) & mask;
    }
    return value;
}

uint32_t WaveFile::bigEndianToInt(char* bytes, int size)
{
    // most significant byte at biggest address
    uint32_t value = 0;
    for (int i = 0; i < size; i++)
    {
        // bitmask is necessary to prevent sign extension
        uint32_t offset = (size - i - 1) * 8;
        uint32_t mask = 0xff << offset;
        value |= bytes[i] << offset;
    }
    return value;
}

std::string WaveFile::intToLittleEndian(int value, int size)
{
    char* bytes = (char*) malloc(size);
    uint32_t mask = (1 << 8) - 1;
    for (int i = 0; i < size; i++)
    {
        bytes[i] = value & mask;
        value >>= 8;
    }
    return std::string(bytes, size);
}