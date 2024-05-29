g++ *.cpp -o windigo

./windigo -1 ../samples/8-bit.wav ../demo/8-bit/-1.wav
./windigo 0 ../samples/8-bit.wav ../demo/8-bit/0.wav
./windigo 1 ../samples/8-bit.wav ../demo/8-bit/1.wav

./windigo -1 ../samples/16-bit.wav ../demo/16-bit/-1.wav
./windigo 0 ../samples/16-bit.wav ../demo/16-bit/0.wav
./windigo 1 ../samples/16-bit.wav ../demo/16-bit/1.wav