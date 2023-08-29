#define _USE_MATH_DEFINES
#include <math.h>
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

/* Parameters */
/* Input */
const int InSampleRate = 2000000; /* 2Mhz */

/* Low Pass Filter */
const int CutOffFrequency = 200000; /* 200Khz */
//const int CutOffFrequency = 5000000; /* 5Mhz */

/* Output */
//const int OutSampleRate = 48000; /* 48KHz */
const int OutSampleRate = 16000; /* 16KHz */
const int OutChannels = 1;

int main()
{
	auto start = std::chrono::high_resolution_clock::now();

	/* Open IQ file */
	std::ifstream inputFile("Input.iq", std::ios::binary);
	inputFile.seekg(0, std::ios::end);

	/* Calculate number of samples and create array */
	ArrayWrapper<std::complex<float>> ComplexSignal(inputFile.tellg() / sizeof(std::complex<float>));

	/* reset stream position */
	inputFile.seekg(0, std::ios::beg);

	/* read data and put it into array */
	printf("Reading complex signal from file\n");
	inputFile.read(reinterpret_cast<char*>(ComplexSignal.data), ComplexSignal.size * sizeof(std::complex<float>));
	inputFile.close();

	/* do a low pass filter on the data */
	printf("Filtering complex signal\n");
	ArrayWrapper<std::complex<float>> filteredSignal = LowPassFilterComplex(ComplexSignal, InSampleRate, CutOffFrequency);
	ComplexSignal.Delete();

	/* down sample the data */
	printf("Down sampling complex signal\n");
	ArrayWrapper<std::complex<float>> downSampledSignal = DownSample(filteredSignal, InSampleRate, OutSampleRate);
	filteredSignal.Delete();

	/* do a FM demodulation on the data */
	printf("FM Demodulating the complex signal\n");
	ArrayWrapper<float> audio = fmDemodulate(downSampledSignal);
	downSampledSignal.Delete();

	/* write the data into a wav file */
	printf("Writing audio signal to file\n");
	WriteData(L"Test.wav", audio.data, audio.size, OutChannels, OutSampleRate);
	audio.Delete();

	//ArrayWrapper<float> audioRead = ReadFile(L"Test.wav");

	auto stop = std::chrono::high_resolution_clock::now();

	printf("Signal processing took: %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

	start = std::chrono::high_resolution_clock::now();

	char* input[8];
	input[0] = (char*)R"(C:\Programing Projects\C++\Project LVATT\build\x64-release\LVATT\LVATT.exe)";
	input[1] = (char*)R"(--threads)";
	input[2] = (char*)R"(14)";
	input[3] = (char*)R"(C:\Programing Projects\C++\Project LVATT\build\x64-release\LVATT\Test.wav)";
	input[4] = (char*)"--model";
	input[5] = (char*)R"(C:\Programing Projects\C++\Project LVATT\models\ggml-medium.bin)";
	input[6] = (char*)"--language";
	input[7] = (char*)"auto";

	TranscribeAudio(8, input);

	stop = std::chrono::high_resolution_clock::now();

	printf("Audio Transcribing took: %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

	printf("Press any button to continue"); std::cin.get();
	return 0;
}