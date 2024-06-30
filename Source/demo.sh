#!/bin/bash

# the following script compiles the project and runs on samples files,
# saving the results into the demo directory

echo "Compiling ..."
g++ -c WaveFile.cpp
g++ -c FourierTransformer.cpp
g++ -c PitchShifter.cpp
g++ demo.cpp -I -o windigo

echo "Processing 8-bit files ..."
mkdir -p ../demo/8-bit
./windigo -1 ../samples/8-bit/a.wav ../demo/8-bit/a-1.wav
./windigo  0 ../samples/8-bit/a.wav ../demo/8-bit/a+0.wav
./windigo  1 ../samples/8-bit/a.wav ../demo/8-bit/a+1.wav

echo "Processing 16-bit files ..."
mkdir -p ../demo/16-bit
./windigo -1 ../samples/16-bit/a.wav ../demo/16-bit/a-1.wav
./windigo  0 ../samples/16-bit/a.wav ../demo/16-bit/a+0.wav
./windigo  1 ../samples/16-bit/a.wav ../demo/16-bit/a+1.wav
./windigo  1 ../samples/16-bit/acoustic-guitar.wav ../demo/16-bit/acoustic-guitar+1.wav
./windigo -1 ../samples/16-bit/clean-guitar.wav ../demo/16-bit/clean-guitar-1.wav

echo "Processing 24-bit files ..."
mkdir -p ../demo/24-bit
./windigo  1 ../samples/24-bit/acoustic-guitar.wav ../demo/24-bit/acoustic-guitar+1.wav
./windigo -1 ../samples/24-bit/clean-guitar.wav ../demo/24-bit/clean-guitar-1.wav

echo "Processing 32-bit files ..."
mkdir -p ../demo/32-bit
./windigo  1 ../samples/32-bit/acoustic-guitar.wav ../demo/32-bit/acoustic-guitar+1.wav
./windigo -1 ../samples/32-bit/clean-guitar.wav ../demo/32-bit/clean-guitar-1.wav
