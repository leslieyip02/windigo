# Examples

This directory contains code for a simple CLI application which demonstrates the `PitchShifter` class. Sample `.wav` files are contained within `samples`, and the pitch shifted files are contained within `results`. The audio files can be inspected in `demo.ipynb`.

### Usage
```
# transforms all samples
bash demo.sh

# OR compile manually
g++ demo.cpp \
    ../Source/FourierTransformer.cpp \
    ../Source/PitchShifter.cpp \
    ../Source/WaveFile.cpp -o windigo

./windigo [step to shift] <input wav> <output wav>
```
