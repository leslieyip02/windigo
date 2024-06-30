#!/bin/bash

# the following script compiles the project
# and runs the pitch shifter on samples files in the samples directory,
# saving the results into the results directory

echo "Compiling ..."
g++ results.cpp \
    ../Source/FourierTransformer.cpp \
    ../Source/PitchShifter.cpp \
    ../Source/WaveFile.cpp -o windigo

echo "Processing 8-bit files ..."
mkdir -p ./demo/8-bit
./windigo -1 ./samples/8-bit/a.wav ./results/8-bit/a-1.wav
./windigo  0 ./samples/8-bit/a.wav ./results/8-bit/a+0.wav
./windigo  1 ./samples/8-bit/a.wav ./results/8-bit/a+1.wav

echo "Processing 16-bit files ..."
mkdir -p ./results/16-bit
./windigo -1 ./samples/16-bit/a.wav ./results/16-bit/a-1.wav
./windigo  0 ./samples/16-bit/a.wav ./results/16-bit/a+0.wav
./windigo  1 ./samples/16-bit/a.wav ./results/16-bit/a+1.wav
./windigo  1 ./samples/16-bit/acoustic-guitar.wav ./results/16-bit/acoustic-guitar+1.wav
./windigo -1 ./samples/16-bit/clean-guitar.wav ./results/16-bit/clean-guitar-1.wav

echo "Processing 24-bit files ..."
mkdir -p ./results/24-bit
./windigo  1 ./samples/24-bit/acoustic-guitar.wav ./results/24-bit/acoustic-guitar+1.wav
./windigo -1 ./samples/24-bit/clean-guitar.wav ./results/24-bit/clean-guitar-1.wav

echo "Processing 32-bit files ..."
mkdir -p ./results/32-bit
./windigo  1 ./samples/32-bit/acoustic-guitar.wav ./results/32-bit/acoustic-guitar+1.wav
./windigo -1 ./samples/32-bit/clean-guitar.wav ./results/32-bit/clean-guitar-1.wav
