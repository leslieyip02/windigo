#include "WaveFile.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#include <bitset>

WaveFile::WaveFile(std::string filename)
{
    // std::ios_base::binary is necessary for windows
    std::ifstream byteStream(filename, std::ios::binary);
    char* headerBuffer = (char*) malloc(PCM_HEADER_SIZE);
    byteStream.read(headerBuffer, PCM_HEADER_SIZE);

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

    // extensible format: https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    if (audioFormat == 0xfffe)
    {
        headerPointer += 2;
        int cbSize = littleEndianToInt(headerPointer, 2); // this is either 0 or 22

        // resize header annd update pointer
        headerBuffer = (char*) realloc(headerBuffer, PCM_HEADER_SIZE + cbSize + 2);
        byteStream.read(&headerBuffer[PCM_HEADER_SIZE], cbSize + 2);
        headerPointer = &headerBuffer[cbSize + 38];

        // check for the fact chunk
        // hex value obtained by hexdump
        int chunkId = bigEndianToInt(headerPointer, 4);
        if (chunkId == 0x66616374)
        {
            headerPointer += 4;
            int chunkSize = littleEndianToInt(headerPointer, 4);

            // resize header and update pointer
            headerBuffer = (char*) realloc(headerBuffer, PCM_HEADER_SIZE + cbSize + chunkSize + 6);
            byteStream.read(&headerBuffer[PCM_HEADER_SIZE + cbSize + 2], chunkSize + 8);
            headerPointer = &headerBuffer[cbSize + chunkSize + 44];
        }
        else
        {
            // keep looking for Subchunk2ID
            int size = PCM_HEADER_SIZE + cbSize + 2;
            while (bigEndianToInt(headerPointer, 4) != 0x64617461)
            {
                int offset = headerPointer - headerBuffer;

                // resize header and update pointer
                headerBuffer = (char*) realloc(headerBuffer, size + 1);
                byteStream.read(&headerBuffer[size], 1);
                headerPointer = &headerBuffer[offset] + 1;
                size++;
            }
            headerPointer -= 2;
        }
    }

    headerPointer += 2; // 36: Subchunk2ID
    assert(bigEndianToInt(headerPointer, 4) == 0x64617461); // data
    headerPointer += 4; // 40: Subchunk2Size
    uint32_t dataSize = littleEndianToInt(headerPointer, 4);
    numSamples = dataSize / numChannels / bytesPerSample;

    // only support PCM and IEEE float formats
    assert(audioFormat == 0x1 || audioFormat == 0x3 || audioFormat == 0xfffe);
    bool pcm = audioFormat == 0x1 || audioFormat == 0xfffe;

    free(headerBuffer);

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
                // lifted from scipy: https://github.com/scipy/scipy/blob/v1.13.1/scipy/io/wavfile.py#L541-L706
                // =====================  ===========  ===========  =============
                //     WAV format            Min          Max       NumPy dtype
                // =====================  ===========  ===========  =============
                // 32-bit integer PCM     -2147483648  +2147483647  int32
                // 24-bit integer PCM     -2147483648  +2147483392  int32
                // 16-bit integer PCM     -32768       +32767       int16
                // 8-bit integer PCM      0            255          uint8
                // =====================  ===========  ===========  =============
                if (bitsPerSample == 8)
                {
                    // values in the range [0, 255]
                    samples[j][i] = (double) rawValue / 255.0 * 2.0 - 1.0;
                }
                else if (bitsPerSample == 16)
                {
                    // values in the range [-32768, 32767]
                    samples[j][i] = (double) static_cast<int16_t>(rawValue) / 32768.0;
                }
                else if (bitsPerSample == 24)
                {
                    // values in the range [-2147483648, 2147483392]
                    // right shift so the range becomes [-2147483648, 2147483647]
                    // static_cast is necessary because the values are stored in 2's complement
                    samples[j][i] = (double) ((static_cast<int32_t>(rawValue << 8)) / 2147483648.0);
                }
                else if (bitsPerSample == 32)
                {
                    // values in the range [-2147483648, 2147483647]
                    samples[j][i] = (double) ((static_cast<int32_t>(rawValue)) / 2147483648.0);
                }
            }
            else
            {
                // if IEEE, the float should already be [-1.0, 1,0)
                samples[j][i] = static_cast<double>(rawValue);
            }
        }
    }
    free(sampleBuffer);
}

void WaveFile::write(std::string filename)
{
    // std::ios_base::binary is necessary for windows
    std::ofstream output(filename, std::ios_base::binary);

    // clamp to 16 bits so the header is easier to write
    if (bitsPerSample > 16)
    {
        bitsPerSample = 16;
    }

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
                samples[j][i] = samples[j][i] * 32768.0;
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
        value |= (bytes[i] << offset) & mask;
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