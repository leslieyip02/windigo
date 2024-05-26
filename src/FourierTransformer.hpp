#ifndef FOURIERTRANSFORMER_HEADER
#define FOURIERTRANSFORMER_HEADER

#include "WaveFile.hpp"

class FourierTransformer
{
public:
    void fft(Channel input, uint32_t numSamples);
    uint32_t reverseBits(uint32_t num, uint8_t log2n);

private:
    uint8_t padInput(Channel& input, uint32_t numSamples);
    uint32_t modulo(uint32_t num, uint8_t modulus);
};

#endif