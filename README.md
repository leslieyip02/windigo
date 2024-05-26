# Windigo

*Project under the NUS SoC Orbital Program*

Team ID: 6466 (Apollo 11)

### Usage

#### Build C++ executable

The main project is contained within `src`. For now, `main.cpp` performs FFT on a sample audio file, performs inverse FFT, and outputs the corresponding amplitude of each frequency bin into a `.csv` file.

```
cd src
g++ *.cpp -o windigo
./windigo [input-filename] [output-filename]
```

#### Set up Python environment

Python will be used to conduct preliminary analysis on the audio files.

```
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Project Structure

```
+-- windigo/
|   +-- demo/ (meant for sanity checks)
|   |   +-- example.wav
|   |   +-- example.csv
|   |
|   +-- samples/ (contains sample audio to test on)
|   |   +-- 8-bit.wav
|   |   +-- 16-bit.wav
|   |
|   +-- src/ (contains all C++ source code)
|   |   + xxx.cpp
|   |   + xxx.hpp
|   |
|   +-- fft.ipynb (meant for quick experimentation)
|   +-- requirements.txt (python dependencies)
```