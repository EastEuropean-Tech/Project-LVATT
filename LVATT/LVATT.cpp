#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <format>
#include <chrono>

#include <conio.h>

#include <DspFilters/Dsp.h>
#include "Headers/WAV.hpp"

/* Parameters */
/* Input */
const int InSampleRate = 2000000; /* 2Mhz */

/* Low Pass Filter */
//const int CutOffFrequency = 200000; /* 200Khz */
const int CutOffFrequency = 5000000; /* 5Mhz */

/* Frequency Demodulation */
const int CarrierFrequency = 446155200; /* 446.155Mhz */

/* Output */
const int OutSampleRate = 48000; /* 48KHz */
const int OutChannels = 1;

void fmDemodulate(std::complex<float>* complexSignal, const size_t& complexSignalSize, float* outArray)
{
	for (size_t i = 1; i < complexSignalSize; i++)
	{
		outArray[i] = std::arg(complexSignal[i] * std::conj(complexSignal[i - 1]));
	}
}

void LowPassFilterComplex(std::complex<float>* inputComplexSignal, size_t inputArraySize, std::complex<float>* outFilteredComplexSignal)
{
	/* set up a simple filter using the Chebyshev's type 2 low pass filter with 2 channels (inPhase and quadrature) */
	Dsp::SimpleFilter<Dsp::ChebyshevII::LowPass<3>, 2> filter;
	filter.setup(3, InSampleRate, CutOffFrequency, 1);

	float* inPhase = new float[inputArraySize];
	float* quadrature = new float[inputArraySize];

	/* separate the complex signal into 2 "channels" */
	for (size_t i = 1; i < inputArraySize; ++i)
	{
		inPhase[i] = inputComplexSignal[i].real();
		quadrature[i] = inputComplexSignal[i].imag();
	}

	float* complexSignalChannels[2];
	complexSignalChannels[0] = inPhase;
	complexSignalChannels[1] = quadrature;

	/* then process the 2 "channels" */
	filter.process(inputArraySize, complexSignalChannels);

	/* combine the 2 channels into a complex signal */
	for (size_t i = 1; i < inputArraySize; i++)
	{
		outFilteredComplexSignal[i] = std::complex<float>(inPhase[i], quadrature[i]);
	}
}

void DownSample(std::complex<float>* inputComplexSignal, const size_t& inputArraySize, std::complex<float>** downSampleComplexSignal, size_t* outArraySize)
{
	int DecimateIndex = (InSampleRate / OutSampleRate);
	(*outArraySize) = 0;

	(*downSampleComplexSignal) = new std::complex<float>[inputArraySize / DecimateIndex];

	for (int i = 0; i < inputArraySize; i += DecimateIndex)
	{
		(*downSampleComplexSignal)[(*outArraySize)] = inputComplexSignal[i];
		(*outArraySize)++;
	}
}

int main()
{
	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();

	/* Open IQ file */
	std::ifstream inputFile("Input.iq", std::ios::binary);
	inputFile.seekg(0, std::ios::end);

	/* Calculate number of samples */
	size_t inSampleCount = inputFile.tellg() / sizeof(std::complex<float>);

	/* reset stream position */
	inputFile.seekg(0, std::ios::beg);

	std::complex<float>* ComplexSignal = new std::complex<float>[inSampleCount];

	/* read data and put it into array */
	printf("Reading complex signal from file\n");
	inputFile.read(reinterpret_cast<char*>(ComplexSignal), inSampleCount * sizeof(std::complex<float>));
	inputFile.close();

	/* LOW PASS FILTER */
	/*
	A low pass filter will remove all frequency above its cut off frequency,
	or you could say it only allows frequencies below its cut off frequency through
	*/
	printf("Filtering complex signal\n");
	LowPassFilterComplex(ComplexSignal, inSampleCount, ComplexSignal);

	std::complex<float>* downSampledComplexSignal = nullptr;
	size_t outSampleCount;
	printf("Down sampling complex signal\n");
	DownSample(ComplexSignal, inSampleCount, &downSampledComplexSignal, &outSampleCount);
	delete[] ComplexSignal;

	/* NARROW BAND FREQUENCY DEMODULATION */
	/*
	takes in the filtered data and demodulates. so it separates the carrier signal from the info signal.
	implement below
	*/

	float* audio = new float[outSampleCount];
	printf("FM Demodulating the complex signal\n");
	fmDemodulate(downSampledComplexSignal, outSampleCount, audio);

	printf("Writing audio signal to file\n");
	WriteData(L"Test.wav", audio, outSampleCount, OutChannels, OutSampleRate);

	delete[] downSampledComplexSignal;
	delete[] audio;

	std::chrono::steady_clock::time_point stop = std::chrono::high_resolution_clock::now();

	printf("overall, took: %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

	printf("Press any button to continue"); _getch();
	return 0;
}