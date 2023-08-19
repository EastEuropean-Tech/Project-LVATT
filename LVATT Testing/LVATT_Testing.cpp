#include <NosLib/Console.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <format>
#include <sndfile.h>

/* Parameters */
/* Output */
const int OutSampleRate = 48000; /* 48KHz */
const int OutChannels = 1;

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
    NosLib::Console::InitializeModifiers::EnableANSI();

	/* Open IQ file */
	std::ifstream inputFile("input.iq", std::ios::binary);
	inputFile.seekg(0, std::ios::end);

	/* Calculate number of samples */
	size_t numberOfSamples = inputFile.tellg() / sizeof(std::complex<float>);

	std::complex<float>* iqData = new std::complex<float>;

	/* read data and put it into array */
	inputFile.read(reinterpret_cast<char*>(iqData), numberOfSamples * sizeof(std::complex<float>));
	inputFile.close();

	/* LOW PASS FILTER */
	/*
	A low pass filter will remove all frequency above its cut off frequency,
	or you could say it only allows frequencies below its cut off frenquency through
	implement below
	*/

	/* NARROW BAND FREQUENCE DEMODULATION */
	/*
	takes in the filtered data and demodulates. so it separates the carrier signal from the info singal.
	implement below
	*/

	/* write audio data into wav file */
	SF_INFO wavInfo;
	wavInfo.samplerate = OutSampleRate;
	wavInfo.channels = OutChannels;
	wavInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

	SNDFILE* wavFile = sf_open("output.wav", SFM_WRITE, &wavInfo);
	//sf_writef_float(wavFile, reinterpret_cast<float*>(modulatedSamples), numSamples / decimation);
	sf_close(wavFile);

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}