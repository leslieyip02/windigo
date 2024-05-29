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
    std::vector<double> omegaBins(frameSize);
    for (int k = 0; k < frameSize; k++)
    {
        omegaBins[k] = 2 * M_PI * k / frameSize;
    }

    for (int channel = 0; channel < file.numChannels; channel++)
    {
        // zero pad both ends of input
        // so that overlap addition of the first and last few frames works properly
        int analysisPadSize = analysisHopSize * (overlapFactor - 1);
        int inputSize = file.numSamples + 2 * analysisPadSize;

        // pad input so that frames will line up nicely
        int endFramePad = inputSize - (int) (inputSize / analysisHopSize) * analysisHopSize;
        inputSize += endFramePad;
        std::vector<std::complex<double>> input(inputSize);
        for (int k = 0; k < file.numSamples; k++)
        {
            input[analysisPadSize + k] = std::complex<double>(file.samples[channel][k]);
        }

        // output size is scaled according to input size since it stores the same audio
        // the pitch shift can then be achieved by resampling
        int numFrames = inputSize / analysisHopSize - (overlapFactor - 1);
        int outputSize = inputSize * scale;
        std::vector<double> output(outputSize);

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
                frame[k] = input[left + k] * window[k] / std::sqrt(((double) frameSize / analysisHopSize) / 2.0);
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
                double deltaPhase = phase - phases[k] - omegaBins[k] * analysisHopSize;

                // constrain phase difference to [-π, π]
                // std::fmod doesn't work with negative numbers, so make this positive first
                if (deltaPhase < 0)
                {
                    deltaPhase += std::ceil(-deltaPhase / (2.0 * M_PI)) * 2.0 * M_PI;
                }
                deltaPhase = std::fmod(deltaPhase + M_PI, 2.0 * M_PI) - M_PI;
                phases[k] = phase;

                double trueFrequency = omegaBins[k] + deltaPhase / analysisHopSize;
                cumulativePhases[k] += trueFrequency * synthesisHopSize;

                buffer[k] = magnitude * std::complex<double>(
                    std::cos(cumulativePhases[k]),
                    std::sin(cumulativePhases[k])
                );
            }

            // apply window
            transformed = transformer.ifft(buffer, frameSize);
            left = i * synthesisHopSize;
            for (int k = 0; k < frameSize; k++)
            {
                output[left + k] += transformed[k].real() * window[k];
            }
        }

        // remove scaled padding
        int synthesisPadSize = synthesisHopSize * (overlapFactor - 1);
        int unpaddedOutputSize = outputSize - 2 * synthesisPadSize;

        // resample output vector with linear interpolation
        // https://paulbourke.net/miscellaneous/interpolation/
        for (int i = 0; i < file.numSamples; i++)
        {
            double x = synthesisPadSize + (double) i / file.numSamples * unpaddedOutputSize;
            double y1 = output[std::floor(x)];
            double y2 = output[std::ceil(x)];
            double ratio = x - std::floor(x);
            file.samples[channel][i] = y1 * (1.0 - ratio) + y2 * ratio;
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
