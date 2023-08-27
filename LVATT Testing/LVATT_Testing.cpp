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

void fmDemodulate(std::complex<float>* complexSignal, const size_t& complexSignalSize, const float& sampleRate, const float& carrierFrequency, float* outArray)
{
	// Calculate carrier wave parameters
	float angularFrequency = 2.0 * M_PI * carrierFrequency / sampleRate;
	std::complex <float> carrier(std::cos(angularFrequency), -std::sin(angularFrequency)); // Complex carrier

	std::complex<float> prevSample = complexSignal[0];

	for (size_t i = 1; i < complexSignalSize; i++)
	{
		std::complex<float> demodulatedSample = complexSignal[i] * std::conj(prevSample);

		float demodulatedValue = std::arg(demodulatedSample); // Extract phase of demodulated sample
		outArray[i] = demodulatedValue;

		prevSample = complexSignal[i];
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

void ListFloatValues(std::complex<float>* inComplexArray,size_t arraySize)
{
	int stepSize = arraySize / 1000; /* 1000 samples shown */

	for (int i = 0; i < arraySize; i += stepSize)
	{
		wprintf(L"%f : %f\n", inComplexArray[i].real(), inComplexArray[i].imag());
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

	//TestWorking();

	/* Open IQ file */
	std::ifstream inputFile("Input.iq", std::ios::binary);
	inputFile.seekg(0, std::ios::end);

	/* Calculate number of samples */
	size_t inSampleCount = inputFile.tellg() / sizeof(std::complex<float>);

	/* reset stream position */
	inputFile.seekg(0, std::ios::beg);

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

	ListFloatValues(filteredComplexSignal, inSampleCount);
	wprintf(L"===============================================\n");
	wprintf(L"===============================================\n");
	wprintf(L"===============================================\n");
	ListFloatValues(downSampledComplexSignal, outSampleCount);

	/* NARROW BAND FREQUENCY DEMODULATION */
	/*
	takes in the filtered data and demodulates. so it separates the carrier signal from the info signal.
	implement below
	*/

	float* audio = new float[outSampleCount];
	fmDemodulate(downSampledComplexSignal, outSampleCount, OutSampleRate, CarrierFrequency, audio);

	/* write audio data into wav file */
	SndfileHandle writeWavFile("output.wav",SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, OutChannels, OutSampleRate);

	writeWavFile.writef(audio, outSampleCount);

	ListFloatValues(audio, outSampleCount);

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}