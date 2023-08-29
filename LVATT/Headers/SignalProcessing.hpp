#include "Common.hpp"

/// <summary>
/// Do a low pass filter on a complex signal
/// </summary>
/// <param name="inputComplexSignal">- complex signal to lowpass</param>
/// <param name="sampleRate">- signal's sample rate</param>
/// <param name="cutOffFrequency">- frequency used for low pass</param>
/// <returns>low passed signal</returns>
ArrayWrapper<std::complex<float>> LowPassFilterComplex(ArrayWrapper<std::complex<float>> inputComplexSignal, const size_t& sampleRate, const size_t& cutOffFrequency)
{
	/* set up a simple filter using the Chebyshev's type 2 low pass filter with 2 channels (inPhase and quadrature) */
	Dsp::SimpleFilter<Dsp::ChebyshevII::LowPass<3>, 2> filter;
	filter.setup(3, sampleRate, cutOffFrequency, 1);

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
ArrayWrapper<float> IQtoAudio(const std::string& iqFileName, const size_t& fileSampleRate, const size_t& cutOffFrequency, const size_t& outSampleRate)
{
	/* Open IQ file */
	std::ifstream inputFile(iqFileName, std::ios::binary);
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
	ArrayWrapper<std::complex<float>> filteredSignal = LowPassFilterComplex(ComplexSignal, fileSampleRate, cutOffFrequency);
	ComplexSignal.Delete();

	/* down sample the data */
	printf("Down sampling complex signal\n");
	ArrayWrapper<std::complex<float>> downSampledSignal = DownSample(filteredSignal, fileSampleRate, outSampleRate);
	filteredSignal.Delete();

	/* do a FM demodulation on the data */
	printf("FM Demodulating the complex signal\n");
	ArrayWrapper<float> audio = fmDemodulate(downSampledSignal);
	downSampledSignal.Delete();

	return audio;
}