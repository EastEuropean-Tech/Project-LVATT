#include "Common.hpp"

ArrayWrapper<float> fmDemodulate(ArrayWrapper<std::complex<float>> complexSignal)
{
	ArrayWrapper<float>	outArray(complexSignal.size);

	for (size_t i = 1; i < complexSignal.size; i++)
	{
		outArray[i] = std::arg(complexSignal[i] * std::conj(complexSignal[i - 1]));
	}

	return outArray;
}

ArrayWrapper<std::complex<float>> LowPassFilterComplex(ArrayWrapper<std::complex<float>> inputComplexSignal, const size_t& sampleRate, const size_t& cutOffFrequency)
{
	/* set up a simple filter using the Chebyshev's type 2 low pass filter with 2 channels (inPhase and quadrature) */
	Dsp::SimpleFilter<Dsp::ChebyshevII::LowPass<3>, 2> filter;
	filter.setup(3, sampleRate, cutOffFrequency, 1);

	float* inPhase = new float[inputComplexSignal.size];
	float* quadrature = new float[inputComplexSignal.size];

	/* separate the complex signal into 2 "channels" */
	for (size_t i = 1; i < inputComplexSignal.size; ++i)
	{
		inPhase[i] = inputComplexSignal[i].real();
		quadrature[i] = inputComplexSignal[i].imag();
	}

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

ArrayWrapper<std::complex<float>> DownSample(ArrayWrapper<std::complex<float>> inputComplexSignal, const size_t& currentSampleRate, const size_t& targetSampleRate)
{
	if (currentSampleRate < targetSampleRate)
	{
		throw std::invalid_argument("current sample rate has to be more then the target sample rate");
		return ArrayWrapper<std::complex<float>>();
	}

	int DecimateIndex = (currentSampleRate / targetSampleRate);

	ArrayWrapper<std::complex<float>> outArray(inputComplexSignal.size / DecimateIndex);

	for (int i = 0; i < inputComplexSignal.size; i += DecimateIndex)
	{
		outArray[outArray.iterator] = inputComplexSignal[i];
		outArray.iterator++;
	}

	return outArray;
}