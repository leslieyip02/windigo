// PitchShifter original main for demo (no Juce UI)
#include "../Source/FourierTransformer.hpp"
#include "../Source/PitchShifter.hpp"
#include "../Source/WaveFile.hpp"

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv)
{
    int steps = argc >= 2 ? std::stoi(argv[1]) : 0;
    std::string inputFilename = argc >= 3 ? argv[2] : "samples/8-bit.wav";
    std::string outputFilename = argc >= 4 ? argv[3] : "output.wav";

    WaveFile file = WaveFile(inputFilename);
    PitchShifter shifter = PitchShifter(8192, 4);
    shifter.shift(file, steps);
    file.write(outputFilename);

    return 0;
}
