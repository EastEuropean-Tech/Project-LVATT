#include <NosLib/Console.hpp>

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

/* Output */
const int OutSampleRate = 48000; /* 48KHz */
const int OutChannels = 1;

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
    NosLib::Console::InitializeModifiers::EnableANSI();

	SndfileHandle readWavFile("input.wav");

	// Print some information about the file
	std::wcout << L"Sample rate: " << readWavFile.samplerate() << std::endl;
	std::wcout << L"Channels: " << readWavFile.channels() << std::endl;
	std::wcout << L"Samples: " << readWavFile.frames() << std::endl;

	// Allocate memory for the samples
	float* samples = new float[readWavFile.frames()];
	float** channels = &samples;

	readWavFile.readf(samples, readWavFile.frames());


	/* LOW PASS FILTER */
	/*
	A low pass filter will remove all frequency above its cut off frequency,
	or you could say it only allows frequencies below its cut off frequency through
	*/

	Dsp::SimpleFilter<Dsp::Bessel::LowPass<3>, 1> filter;
	filter.setup(3, InSampleRate, CutOffFrequency);
	filter.process(readWavFile.frames(), channels);

	/* NARROW BAND FREQUENCY DEMODULATION */
	/*
	takes in the filtered data and demodulates. so it separates the carrier signal from the info signal.
	implement below
	*/

	/* write audio data into wav file */
	SndfileHandle writeWavFile("output.wav",SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, OutChannels, OutSampleRate);

	writeWavFile.writef(samples, readWavFile.frames());

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}


/* previous code */

///* Open IQ file */
//std::ifstream inputFile("input.iq", std::ios::binary);
//inputFile.seekg(0, std::ios::end);
//
///* Calculate number of samples */
//size_t inSampleCount = inputFile.tellg() / sizeof(std::complex<float>);
//size_t outSampleCount = inSampleCount / (InSampleRate / OutSampleRate);
//
//std::wcout << inSampleCount << std::endl;
//std::wcout << outSampleCount << std::endl;
//
//std::complex<float>* iqData = new std::complex<float>[inSampleCount];
//
///* read data and put it into array */
//inputFile.read(reinterpret_cast<char*>(iqData), inSampleCount * sizeof(std::complex<float>));
//inputFile.close();
//
//float* output = new float[inSampleCount];
//
//for (int i = 0; i < inSampleCount; i++)
//{
//	if (i % (InSampleRate / OutSampleRate) != 0)
//	{
//		continue;
//	}
//
//	output[i] = iqData[i].real();
//}