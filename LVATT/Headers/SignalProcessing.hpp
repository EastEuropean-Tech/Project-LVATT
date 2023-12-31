#include <string>
#include <complex>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <DspFilters/Dsp.h>
#include "Common.hpp"

#include "../NosLib/String.hpp"

struct InputFile
{
	std::string FilePath;
	size_t FileSampleRate = 2000000;
	size_t CutOffFrequency = 200000;
};

/// <summary>
/// Do a low pass filter on a complex signal
/// </summary>
/// <param name="inputComplexSignal">- complex signal to lowpass</param>
/// <param name="sampleRate">- signal's sample rate</param>
/// <param name="cutOffFrequency">- frequency used for low pass</param>
/// <returns>low passed signal</returns>
ArrayWrapper<std::complex<float>> LowPassFilterComplex(ArrayWrapper<std::complex<float>> inputComplexSignal, const size_t& sampleRate, const size_t& CutOffFrequency)
{
	/* set up a simple filter using the Chebyshev's type 2 low pass filter with 2 channels (inPhase and quadrature) */
	Dsp::SimpleFilter<Dsp::ChebyshevII::LowPass<3>, 2> filter;
	filter.setup(3, sampleRate, CutOffFrequency, 1);

	/* separate complex signal into 2 "channels" */
	float* inPhase = new float[inputComplexSignal.size];
	float* quadrature = new float[inputComplexSignal.size];

	/* separate the complex signal into 2 "channels" */
	for (size_t i = 1; i < inputComplexSignal.size; ++i)
	{
		inPhase[i] = inputComplexSignal[i].real();
		quadrature[i] = inputComplexSignal[i].imag();
	}

	/* put the 2 "channels" into a channel array */
	float* complexSignalChannels[2];
	complexSignalChannels[0] = inPhase;
	complexSignalChannels[1] = quadrature;

	/* then process the 2 "channels" */
	filter.process(inputComplexSignal.size, complexSignalChannels);

	ArrayWrapper<std::complex<float>> outArray(inputComplexSignal.size);

	/* combine the 2 channels into a complex signal */
	for (size_t i = 1; i < inputComplexSignal.size; i++)
	{
		outArray[i] = std::complex<float>(inPhase[i], quadrature[i]);
	}

	return outArray;
}

/// <summary>
/// Down samples a signal
/// </summary>
/// <param name="inputComplexSignal">- signal to downsample</param>
/// <param name="currentSampleRate">- currently used sample rate</param>
/// <param name="targetSampleRate">- wanted sample rate</param>
/// <returns>down sampled complex signal</returns>
ArrayWrapper<std::complex<float>> DownSample(ArrayWrapper<std::complex<float>> inputComplexSignal, const size_t& currentSampleRate, const size_t& targetSampleRate)
{
	/* if target sample rate is more then current sample rate, throw error (you can't up sample this easy) */
	if (currentSampleRate < targetSampleRate)
	{
		throw std::invalid_argument("current sample rate has to be more then the target sample rate");
		return ArrayWrapper<std::complex<float>>();
	}

	/* calculate the decimate index */
	int DecimateIndex = (currentSampleRate / targetSampleRate);

	/* create array which will contain the down sampled signal */
	ArrayWrapper<std::complex<float>> outArray(inputComplexSignal.size / DecimateIndex);

	/* down sample by taking a sample every decimate index */
	for (int i = 0; i < inputComplexSignal.size; i += DecimateIndex)
	{
		outArray[outArray.iterator] = inputComplexSignal[i];
		outArray.iterator++;
	}

	return outArray;
}

/// <summary>
/// takes in a complex signal and FM demodulates it
/// </summary>
/// <param name="complexSignal">- complex signal</param>
/// <returns>array of floats (the audio)</returns>
ArrayWrapper<float> fmDemodulate(ArrayWrapper<std::complex<float>> complexSignal)
{
	ArrayWrapper<float>	outArray(complexSignal.size);

	for (size_t i = 1; i < complexSignal.size; i++)
	{
		outArray[i] = std::arg(complexSignal[i] * std::conj(complexSignal[i - 1]));
	}

	return outArray;
}

/// <summary>
/// Takes in a file name to a IQ file, and returns an audio signal as a float array
/// </summary>
/// <param name="iqFileName">- name to IQ file</param>
/// <returns>array of floats</returns>
ArrayWrapper<float> IQtoAudio(const std::string& iqFilePath, const size_t& FileSampleRate, const size_t& CutOffFrequency, const size_t& outSampleRate)
{
	if (!std::filesystem::exists(iqFilePath)) /* if doesn't exist, just return */
	{
		printf("not file found at: %s\n", iqFilePath.c_str());
		return ArrayWrapper<float>();
	}

	/* Open IQ file */
	std::ifstream inputFile(iqFilePath, std::ios::binary);
	inputFile.seekg(0, std::ios::end);

	/* Calculate number of samples and create array */
	ArrayWrapper<std::complex<float>> ComplexSignal(inputFile.tellg() / sizeof(std::complex<float>));

	/* reset stream position */
	inputFile.seekg(0, std::ios::beg);

	printf("Processing %s\nIn Sample rate: %zuHz\nSamples: %zuHz\nLenght: %fs\nOut Sample rate: %zuHz\n", iqFilePath.c_str(), FileSampleRate, ComplexSignal.size, float(ComplexSignal.size)/float(FileSampleRate), outSampleRate);

	/* read data and put it into array */
	printf("Reading complex signal from file\n");
	inputFile.read(reinterpret_cast<char*>(ComplexSignal.data), ComplexSignal.size * sizeof(std::complex<float>));
	inputFile.close();

	/* do a low pass filter on the data */
	printf("Filtering complex signal\n");
	ArrayWrapper<std::complex<float>> filteredSignal = LowPassFilterComplex(ComplexSignal, FileSampleRate, CutOffFrequency);
	ComplexSignal.Delete();

	/* down sample the data */
	printf("Down sampling complex signal\n");
	ArrayWrapper<std::complex<float>> downSampledSignal = DownSample(filteredSignal, FileSampleRate, outSampleRate);
	filteredSignal.Delete();

	/* do a FM demodulation on the data */
	printf("FM Demodulating the complex signal\n");
	ArrayWrapper<float> audio = fmDemodulate(downSampledSignal);
	downSampledSignal.Delete();

	return audio;
}

/* Input */
const size_t DefaultInSampleRate = 2000000; /* 2Mhz */

/* Low Pass Filter */
const size_t DefaultCutOffFrequency = 200000; /* 200Khz */
//const int CutOffFrequency = 5000000; /* 5Mhz */

ArrayWrapper<InputFile> GatherUserInput()
{
	std::string paths;
	printf("Input path to IQ file\\s [Separate each path with ,]: ");
	std::getline(std::cin, paths);

	NosLib::DynamicArray<std::string> splitOut;
	NosLib::String::Split<char>(&splitOut, paths, ',');

	ArrayWrapper<InputFile> outArray(splitOut.GetItemCount());

	for (int i = 0; i <= splitOut.GetLastArrayIndex(); i++)
	{
		InputFile currentInput;
		currentInput.FilePath = NosLib::String::Trim(splitOut[i]);

		while (true)
		{
			std::string input;
			printf("\nPlease input the sample rate for \"%s\" [Default:%zuHz]: ", splitOut[i].c_str(), DefaultInSampleRate);
			std::getline(std::cin, input);

			if (input.empty())
			{
				printf("Using default value: %zuHz\n", DefaultInSampleRate);
				currentInput.FileSampleRate = DefaultInSampleRate;
				break;
			}

			if (1 == sscanf(input.c_str(), "%zu", &currentInput.FileSampleRate))
			{
				break;
			}

			printf("Input was invalid, try again\n");
		}
		
		while (true)
		{
			std::string input;
			printf("\nPlease input the Cut off frequency for \"%s\" [Default:%zuHz]: ", splitOut[i].c_str(), DefaultCutOffFrequency);
			std::getline(std::cin, input);

			if (input.empty())
			{
				printf("Using default value: %zuHz\n", DefaultCutOffFrequency);
				currentInput.CutOffFrequency = DefaultCutOffFrequency;
				break;
			}

			if (1 == sscanf(input.c_str(), "%zu", &currentInput.CutOffFrequency))
			{
				break;
			}

			printf("Input was invalid, try again\n");
		}

		outArray[i] = currentInput;
	}

	return outArray;
}