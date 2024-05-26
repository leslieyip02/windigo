#ifndef FOURIERTRANSFORMER_HEADER
#define FOURIERTRANSFORMER_HEADER

#include "WaveFile.hpp"

#include <complex>
#include <vector>

class FourierTransformer
{
public:
    std::vector<std::complex<double>> fft(Channel input, uint32_t numSamples);
    std::vector<std::complex<double>> ifft(std::vector<std::complex<double>> input, uint32_t numSamples);
    uint32_t reverseBits(uint32_t num, uint8_t log2n);

private:
    uint8_t padInput(Channel& input, uint32_t numSamples);
    uint32_t modulo(uint32_t num, uint8_t modulus);
};

#endif