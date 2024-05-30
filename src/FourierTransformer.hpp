#ifndef FOURIERTRANSFORMER_HEADER
#define FOURIERTRANSFORMER_HEADER

#include "WaveFile.hpp"

#include <complex>
#include <vector>

#define M_PI 3.14159265358979323846
constexpr std::complex<double> I(0, 1);

class FourierTransformer
{
public:
    std::vector<std::complex<double>> fft(std::vector<std::complex<double>> input, uint32_t numSamples);
    std::vector<std::complex<double>> ifft(std::vector<std::complex<double>> input, uint32_t numSamples);

private:
    uint8_t padInput(std::vector<std::complex<double>>& input, uint32_t numSamples);
    uint32_t reverseBits(uint32_t num, uint8_t log2n);
    uint32_t modulo(uint32_t num, uint8_t modulus);
};

#endif