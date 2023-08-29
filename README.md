# PROJECT LVATT
Purpose built for converting FM IQ to WAV and trascribed text

## What is project LVATT?
LVATT is a project that incorporates DSP, audio manipulation and audio trancribing and translating. It takes in a FM IQ file, then it extracts the audio from the IQ file, It both writes audio into a file and passes it an AI model for transcribing and translating.

## How to build
```bash
cd {root project directory}
mkdir build; cd build
cmake ..
cmake --build . --config Release
```