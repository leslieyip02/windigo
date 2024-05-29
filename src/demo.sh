#!/bin/bash

# the following script compiles the project and runs on samples files,
# saving the results into the demo directory

echo "Compiling ..."
g++ *.cpp -o windigo

echo "Processing 8-bit files ..."
./windigo -1 ../samples/8-bit/a.wav ../demo/8-bit/a-1.wav
./windigo 0 ../samples/8-bit/a.wav ../demo/8-bit/a+0.wav
./windigo 1 ../samples/8-bit/a.wav ../demo/8-bit/a+1.wav
./windigo 1 ../samples/8-bit/acoustic-guitar.wav ../demo/8-bit/acoustic-guitar+1.wav
./windigo -1 ../samples/8-bit/jazz-guitar.wav ../demo/8-bit/jazz-guitar-1.wav

echo "Processing 16-bit files ..."
./windigo -1 ../samples/16-bit/a.wav ../demo/16-bit/a-1.wav
./windigo 0 ../samples/16-bit/a.wav ../demo/16-bit/a+0.wav
./windigo 1 ../samples/16-bit/a.wav ../demo/16-bit/a+1.wav
./windigo 1 ../samples/16-bit/acoustic-guitar.wav ../demo/16-bit/acoustic-guitar+1.wav
./windigo -1 ../samples/16-bit/jazz-guitar.wav ../demo/16-bit/jazz-guitar-1.wav
