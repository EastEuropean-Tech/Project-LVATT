#include <NosLib/Console.hpp>
#include <NosLib/DynamicArray.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <format>
#include <sndfile.hh>
#include <DspFilters/Dsp.h>

/* Parameters */
/* Input */
const int InSampleRate = 2000000; /* 2Mhz */

/* Low Pass Filter */
const int CutOffFrequency = 200000; /* 200Khz */

/* Frequency Demodulation */
const int CarrierFrequency = 446155200; /* 446.155Mhz */

/* Output */
const int OutSampleRate = 48000; /* 48KHz */
const int OutChannels = 1;

// Function to perform FM demodulation
void fmDemodulate(std::complex<float>* complexSignal, const size_t& complexSignalSize, const float& samplingRate, const float& carrierFrequency, float* outArray)
{
	// Calculate the phase difference between adjacent samples
	for (size_t i = 1; i < complexSignalSize; i++)
	{
		std::complex<float> prevSample = complexSignal[i - 1];
		std::complex<float> currentSample = complexSignal[i];

		// Calculate phase difference
		float phaseDifference = std::arg(currentSample * std::conj(prevSample));

		// Calculate instantaneous frequency
		float instantaneousFrequency = (phaseDifference / (2 * M_PI)) * samplingRate;

		// Subtract carrier frequency to get the modulating signal
		float demodulatedValue = instantaneousFrequency - carrierFrequency;

		outArray[i] = demodulatedValue;
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

void DownSample(std::complex<float>* inputComplexSignal, const size_t& inputArraySize, std::complex<float>* downSampleComplexSignal, size_t* outArraySize)
{
	int DecimateIndex = (InSampleRate / OutSampleRate);
	(*outArraySize) = 0;

	for (int i = 0; i < inputArraySize; i+= DecimateIndex)
	{
		downSampleComplexSignal[(*outArraySize)] = inputComplexSignal[i];
		(*outArraySize)++;
	}
}

void ListFloatValues(float* inArray, size_t arraySize)
{
	int stepSize = arraySize / 1000; /* 1000 samples shown */

	for (int i = 0; i < arraySize; i+= stepSize)
	{
		wprintf(L"%f\n", inArray[i]);
	}
}

void TestWorking()
{
	SndfileHandle readWavFile("input.wav");

	float* samples = new float[readWavFile.frames()];

	readWavFile.readf(samples, readWavFile.frames());

	ListFloatValues(samples, readWavFile.frames());
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
    NosLib::Console::InitializeModifiers::EnableANSI();

	TestWorking();

	/* Open IQ file */
	std::ifstream inputFile("input.iq", std::ios::binary);
	inputFile.seekg(0, std::ios::end);

	/* Calculate number of samples */
	size_t inSampleCount = inputFile.tellg() / sizeof(std::complex<float>);

	std::wcout <<L"In Sample Count:\t" << inSampleCount << std::endl;

	NosLib::DynamicArray<std::complex<float>> inputComplexSignal(inSampleCount);

	/* read data and put it into array */
	inputFile.read(reinterpret_cast<char*>(inputComplexSignal.GetArray()), inSampleCount * sizeof(std::complex<float>));
	inputFile.close();

	/* LOW PASS FILTER */
	/*
	A low pass filter will remove all frequency above its cut off frequency,
	or you could say it only allows frequencies below its cut off frequency through
	*/
	std::complex<float>* filteredComplexSignal = new std::complex<float>[inSampleCount];
	LowPassFilterComplex(inputComplexSignal.GetArray(), inSampleCount, filteredComplexSignal);

	std::complex<float>* downSampledComplexSignal = new std::complex<float>[inSampleCount];
	size_t outSampleCount;
	DownSample(filteredComplexSignal, inSampleCount, downSampledComplexSignal, &outSampleCount);
	std::wcout << L"Out Sample Count:\t" << outSampleCount << std::endl;

	/* NARROW BAND FREQUENCY DEMODULATION */
	/*
	takes in the filtered data and demodulates. so it separates the carrier signal from the info signal.
	implement below
	*/

	float* audio = new float[inSampleCount];
	fmDemodulate(downSampledComplexSignal, outSampleCount, OutSampleRate, CarrierFrequency, audio);

	/* write audio data into wav file */
	SndfileHandle writeWavFile("output.wav",SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, OutChannels, OutSampleRate);

	writeWavFile.writef(audio, outSampleCount);

	ListFloatValues(audio, outSampleCount);

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}