#include "FourierTransformer.hpp"
#include "PitchShifter.hpp"
#include "WaveFile.hpp"

#include <cmath>
#include <complex>
#include <vector>

PitchShifter::PitchShifter(int frameSize, int overlapFactor)
{
    // recommended 75% overlap
    this->frameSize = frameSize;
    this->overlapFactor = overlapFactor;
}

void PitchShifter::shift(WaveFile& file, int steps)
{
    // based on https://www.guitarpitchshifter.com/algorithm.html
    // and https://www.guitarpitchshifter.com/matlab.html
    // and Chapter 5 https://www-fourier.ujf-grenoble.fr/~faure/enseignement/musique/documents/chapter_4_musical_theories/2007_Sethares-Rhythm%20and%20Transforms.pdf

    double scale = pow(2.0, (double) steps / 12.0);
    int analysisHopSize = frameSize / overlapFactor;
    int synthesisHopSize = analysisHopSize * scale;
    std::vector<double> window = hanningWindow(frameSize);

    for (int channel = 0; channel < file.numChannels; channel++)
    {
        // zero pad both ends of input
        // so that overlap addition of the first and last few frames works properly
        int padLength = analysisHopSize * (overlapFactor - 1);
        std::vector<std::complex<double>> input(file.numSamples + padLength * 2);
        for (int k = 0; k < file.numSamples; k++)
        {
            input[padLength + k] = std::complex<double>(file.samples[channel][k]);
        }

        // zero pad input so that its length is a power of 2
        int log2n = transformer.padInput(input, input.size());
        int numSamples = 1 << log2n;

        int numFrames = numSamples / analysisHopSize - (overlapFactor - 1);
        std::vector<double> output(numFrames * synthesisHopSize);
        // std::vector<double> output(numSamples);

        std::vector<double> phases(frameSize);
        std::vector<double> cumulativePhases(frameSize);

        for (int i = 0; i < numFrames; i++)
        {
            std::vector<std::complex<double>> frame(frameSize);
            std::vector<std::complex<double>> transformed(frameSize);
            std::vector<std::complex<double>> buffer(frameSize);

            // apply window
            int left = i * analysisHopSize;
            for (int k = 0; k < frameSize; k++)
            {
                frame[k] = input[left + k] * window[k] / std::sqrt(overlapFactor / 2.0);
            }

            // transform to frequency domain
            transformed = transformer.fft(frame, frameSize);
            buffer = std::vector<std::complex<double>>(frameSize);

            for (int k = 0; k < frameSize; k++)
            {
                // std::abs(const std::complex<T>& x) calculates the magnitude of x
                double magnitude = std::abs(transformed[k]);

                // std::arg(const std::complex<T>& x) calculates the phase of x
                double phase = std::arg(transformed[k]);

                // calculate phase difference
                double expectedDeltaPhase = 2.0 * M_PI * k * analysisHopSize / frameSize;
                double deltaPhase = phase - phases[k] - expectedDeltaPhase;
                deltaPhase = std::fmod(deltaPhase + M_PI, 2.0 * M_PI) - M_PI; // constrain to [-π, π]
                phases[k] = phase;

                double trueFrequency = 2.0 * M_PI * k / frameSize + deltaPhase / analysisHopSize;
                cumulativePhases[k] += trueFrequency * synthesisHopSize;

                // shift pitches
                int index = k * scale;
                if (index < frameSize)
                {
                    buffer[index] = magnitude * std::complex<double>(
                        std::cos(cumulativePhases[k]),
                        std::sin(cumulativePhases[k]) * scale
                    );
                }
            }

            // apply window
            // transformed = transformer.ifft(transformed, frameSize);
            transformed = transformer.ifft(buffer, frameSize);

            // left = i * synthesisHopSize;
            for (int k = 0; k < frameSize; k++)
            {
                output[left + k] += transformed[k].real() * window[k];
            }
        }

        // resample output vector back into the channel with linear interpolation
        for (int i = 0; i < file.numSamples; i++)
        {
            file.samples[channel][i] = output[padLength + i];
        }
    }
}

std::vector<double> PitchShifter::hanningWindow(int frameSize)
{
    // based on NumPy implementation: https://github.com/numpy/numpy/blob/v1.26.0/numpy/lib/function_base.py#L3128-L3234
    std::vector<double> window(frameSize);
    int n = 1 - frameSize;
    for (int i = 0; i < frameSize; i++)
    {
        window[i] = 0.5 + 0.5 * cos(M_PI * n / (frameSize - 1.0));
        n += 2;
    }
    return window;
}
