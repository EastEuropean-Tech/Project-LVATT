﻿#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <format>
#include <chrono>

#include <DspFilters/Dsp.h>
#include "Headers/WAV.hpp"

#include <whisper.h>

std::string to_timestamp(int64_t t, bool comma = false) {
	int64_t msec = t * 10;
	int64_t hr = msec / (1000 * 60 * 60);
	msec = msec - hr * (1000 * 60 * 60);
	int64_t min = msec / (1000 * 60);
	msec = msec - min * (1000 * 60);
	int64_t sec = msec / 1000;
	msec = msec - sec * 1000;

	char buf[32];
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d%s%03d", (int)hr, (int)min, (int)sec, comma ? "," : ".", (int)msec);

	return std::string(buf);
}

void whisper_print_segment_callback(struct whisper_context* ctx, struct whisper_state* /*state*/, int n_new, void* user_data) 
{const int n_segments = whisper_full_n_segments(ctx);

	std::string speaker = "";

	int64_t t0 = 0;
	int64_t t1 = 0;

	// print the last n_new segments
	const int s0 = n_segments - n_new;

	if (s0 == 0) {
		printf("\n");
	}

	for (int i = s0; i < n_segments; i++) {

		t0 = whisper_full_get_segment_t0(ctx, i);
		t1 = whisper_full_get_segment_t1(ctx, i);
		printf("[%s --> %s]  ", to_timestamp(t0).c_str(), to_timestamp(t1).c_str());
		const char* text = whisper_full_get_segment_text(ctx, i);

		printf("%s%s", speaker.c_str(), text);

		fflush(stdout);
	}
}

/* Parameters */
/* Input */
const int InSampleRate = 2000000; /* 2Mhz */

/* Low Pass Filter */
//const int CutOffFrequency = 200000; /* 200Khz */
const int CutOffFrequency = 5000000; /* 5Mhz */

/* Frequency Demodulation */
const int CarrierFrequency = 446155200; /* 446.155Mhz */

/* Output */
//const int OutSampleRate = 48000; /* 48KHz */
const int OutSampleRate = 16000; /* 16KHz */
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
	auto start = std::chrono::high_resolution_clock::now();

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

	struct whisper_context* ctx = whisper_init_from_file(R"(C:\Programing Projects\C++\Project LVATT\models\ggml-large.bin)");

	if (ctx == nullptr)
	{
		printf("Error creating whisper context\n");
		return -1;
	}

	whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

	wparams.strategy = WHISPER_SAMPLING_GREEDY;
	wparams.n_threads = std::min(4, (int32_t)std::thread::hardware_concurrency());
	wparams.n_max_text_ctx = 16384;
	wparams.offset_ms = 0;
	wparams.duration_ms = 0;
	wparams.translate = false;
	wparams.no_context = true;
	wparams.single_segment = false;
	wparams.print_special = false;
	wparams.print_progress = false;
	wparams.print_realtime = false;
	wparams.print_timestamps = true;
	wparams.token_timestamps = false;
	wparams.thold_pt = 0.01f;
	wparams.thold_ptsum = 0.01f;
	wparams.max_len = 0;
	wparams.split_on_word = false;
	wparams.max_tokens = 0;
	wparams.speed_up = false;
	wparams.debug_mode = false;
	wparams.audio_ctx = 0;
	wparams.tdrz_enable = false; // [TDRZ]
	wparams.initial_prompt = "";
	wparams.language = "auto";
	wparams.detect_language = false;
	wparams.suppress_blank = true;
	wparams.suppress_non_speech_tokens = false;
	wparams.greedy.best_of = 2;
	wparams.beam_search.beam_size = -1;
	wparams.new_segment_callback = whisper_print_segment_callback;


	fprintf(stderr, "%s: processing '%s' (%d samples, %.1f sec), %d threads, %d processors, lang = %s, task = %s, %stimestamps = %d ...\n",
		__func__, "abc", int(outSampleCount), float(outSampleCount) / WHISPER_SAMPLE_RATE,
		wparams.n_threads, 2,
		wparams.language,
		wparams.translate ? "translate" : "transcribe",
		wparams.tdrz_enable ? "tdrz = 1, " : "",
		wparams.print_timestamps ? 1 : 0);

	if (whisper_full_parallel(ctx, wparams, audio, outSampleCount, 2) != 0)
	{
		fprintf(stderr, "failed to process audio\n");
		return 10;
	}
	whisper_free(ctx);

	delete[] downSampledComplexSignal;
	delete[] audio;


	auto stop = std::chrono::high_resolution_clock::now();

	printf("overall, took: %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());

	printf("Press any button to continue"); std::cin.get();
	return 0;
}