#include "FourierTransformer.hpp"

#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

#define M_PI 3.14159265358979323846
constexpr std::complex<double> I(0, 1);

// adapted from C++ Cookbook
void FourierTransformer::fft(Channel input, uint32_t numSamples)
{

    // resize input so that its length is a power of 2
    int log2n = padInput(input, numSamples);
    numSamples = 1 << log2n;

    std::vector<std::complex<double>> output(numSamples);
    for (int i = 0; i < numSamples; i++)
    {
        output[reverseBits(i, log2n)] = input[i];
    }

    // using butterfly computations
    // refer to page 6 of https://www.cs.cmu.edu/afs/andrew/scs/cs/15-463/2001/pub/www/notes/fourier/fourier.pdf
    for (int r = 1; r <= log2n; r++)
    {
        int butterflyCoefficient = 1 << r;
        int previousCoefficient = butterflyCoefficient >> 1;
        std::complex<double> alpha(1, 0);
        std::complex<double> w = exp(-I * (M_PI / previousCoefficient));
        for (int j = 0; j < previousCoefficient; j++)
        {
            for (int k = j; k < numSamples; k += butterflyCoefficient)
            {
                std::complex<double> p = output[k];
                std::complex<double> q = alpha * output[k + previousCoefficient];
                output[k] = p + q;
                output[k + previousCoefficient] = p - q;
            }
            alpha *= w;
        }
    }

    // pipe into csv (for now)
    std::cout << "input,output(real),output(imaginary)\n";
    for (int i = 0; i < numSamples; i++)
    {
        std::cout << input[i] << "," << output[i].real() << "," << output[i].imag() << "\n";
    }
}

uint8_t FourierTransformer::padInput(Channel& input, uint32_t numSamples)
{
    // TODO: add unit test
    uint32_t inputSize = 1;
    uint32_t log2n = 0;
    while (inputSize < numSamples)
    {
        inputSize <<= 1;
        log2n++;
    }

    // pad input with zeros
    input.resize(inputSize, 0.0);
    return log2n;
}

uint32_t FourierTransformer::reverseBits(uint32_t num, uint8_t log2n)
{
    // TODO: add unit test
    // reverse bits
    // i.e. 0b1010 becomes 0b0101
    uint32_t result = 0;
    for (int i = 0; i < log2n; i++)
    {
        if (num & 1)
        {
            result |= (1 << (log2n - i - 1));
        }
        num >>= 1;
    }
    return result;
}