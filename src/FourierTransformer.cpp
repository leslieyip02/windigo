#include "FourierTransformer.hpp"

#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>
#include <vector>

#define M_PI 3.14159265358979323846
constexpr std::complex<double> I(0, 1);

void FourierTransformer::fft(Channel input, uint32_t numSamples)
{
    // resize input so that its length is a power of 2
    int log2n = padInput(input, numSamples);
    numSamples = 1 << log2n;

    std::vector<std::complex<double>> output(numSamples);
    std::vector<std::complex<double>> buffer(numSamples);
    for (int i = 0; i < numSamples; i++)
    {
        output[reverseBits(i, log2n)] = input[i];
    }

    // using butterfly computations
    // refer to page 6 of https://www.cs.cmu.edu/afs/andrew/scs/cs/15-463/2001/pub/www/notes/fourier/fourier.pdf
    // W = e^(-i * (2π / N))
    std::complex<double> w = exp(-I * (M_PI / (numSamples >> 1)));
    for (int i = 0; i < log2n; i++)
    {
        int offset = 1 << i;

        // group together computations
        // groups of size 2, 4, 8, ...
        int numGroups = 1 << (log2n - 1 - i);
        int groupSize = 1 << (i + 1);
        for (int j = 0; j < numGroups; j++)
        {
            // for downward pointing arrows (i.e. p)
            for (int k = 0; k < offset; k++)
            {
                int index = j * groupSize + k;
                buffer[index] += output[index]; // p
                buffer[index + offset] += output[index]; // p
            }

            // for upward pointing arrows (i.e. q)
            for (int k = 0; k < offset; k++)
            {
                int index = j * groupSize + k + offset;

                // at stage i of computation,
                // the coefficients of q, alpha = w^x,
                // where x follows the arithmetic progression
                // with common difference N / 2^(i + 1) modulo N
                //
                // e.g. at stage i = 0 for an input of size N = 8,
                // the sequence will be 0, 4, 8, 12, ...
                // the sequence modulo 8 is 0, 4, 0, 4, ...
                int commonDifference = numGroups;

                // upward arrow
                int upExponent = modulo((index - offset) * commonDifference, numSamples);
                std::complex<double> upAlpha = pow(w, upExponent);
                buffer[index - offset] += upAlpha * output[index];

                // horizontal arrow
                int downExponent = modulo(index * commonDifference, numSamples);
                std::complex<double> downAlpha = pow(w, downExponent);
                buffer[index] += downAlpha * output[index];
            }
        }

        output.swap(buffer);
        std::fill(buffer.begin(), buffer.end(), 0.0);
    }

    // output to csv (for now)
    std::ofstream csv("../demo/fft.csv");
    csv << "input,output(real),output(imaginary)\n";
    for (int i = 0; i < numSamples; i++)
    {
        csv << input[i] << "," << output[i].real() << "," << output[i].imag() << "\n";
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

// only works if modulus is a power of 2
uint32_t FourierTransformer::modulo(uint32_t num, uint8_t modulus)
{
    // TODO: add unit test
    return num & (modulus - 1);
}