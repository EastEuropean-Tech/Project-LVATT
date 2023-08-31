#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <format>
#include <chrono>

#include <DspFilters/Dsp.h>

#include "Headers/WAV.hpp"
#include "Headers/SignalProcessing.hpp"
#include "Headers/AudioTranscribing.hpp"


/* Output */
//const int OutSampleRate = 48000; /* 48KHz */
const int OutSampleRate = 16000; /* 16KHz */ /* Whisper requires the audio to be of sample rate 16KHz */
const int OutChannels = 1;

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	ArrayWrapper<InputFile> files = GatherUserInput();

	for (int i = 0; i < files.size; i++)
	{
		/* input and demodulate IQ file */
		ArrayWrapper<float> audio = IQtoAudio(files[i].FilePath, files[i].FileSampleRate, files[i].CutOffFrequency, OutSampleRate);

		/* write the data into a wav file */
		printf("Writing audio signal to file\n");
		WriteData(files[i].FilePath.substr(0,files[i].FilePath.find_last_of('.')) + ".wav", audio.data, audio.size, OutChannels, OutSampleRate);

		auto stop = std::chrono::high_resolution_clock::now();

		printf("Signal processing took: %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

		start = std::chrono::high_resolution_clock::now();

		/* take in the data and pass it to whisper for transcribing */
		TranscribeAudio(audio, R"(ggml-medium.bin)");
		audio.Delete();

		stop = std::chrono::high_resolution_clock::now();

		printf("Audio Transcribing took: %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
	}

	files.Delete();
	FreeWhisperContext();

	printf("Press any button to continue"); std::cin.get();
	return 0;
}