#pragma once
#include "Common.hpp"

#include "whisper.h"

#include <cmath>
#include <fstream>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include <cstring>

//  500 -> 00:05.000
// 6000 -> 01:00.000
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

int timestamp_to_sample(int64_t t, int n_samples) {
	return std::max(0, std::min((int)n_samples - 1, (int)((t * WHISPER_SAMPLE_RATE) / 100)));
}

void whisper_print_segment_callback(struct whisper_context* ctx, struct whisper_state* /*state*/, int n_new, void* user_data)
{
	const int n_segments = whisper_full_n_segments(ctx);

	std::string speaker = "";

	int64_t t0 = 0;
	int64_t t1 = 0;

	// print the last n_new segments
	const int s0 = n_segments - n_new;

	if (s0 == 0) {
		printf("\n");
	}

	for (int i = s0; i < n_segments; i++)
	{
		t0 = whisper_full_get_segment_t0(ctx, i);
		t1 = whisper_full_get_segment_t1(ctx, i);

		printf("[%s --> %s]  %s\n", to_timestamp(t0).c_str(), to_timestamp(t1).c_str(), whisper_full_get_segment_text(ctx, i));

		fflush(stdout);
	}
}

int TranscribeAudio()
{
	// whisper init

	struct whisper_context* ctx = whisper_init_from_file(R"(C:\Programing Projects\C++\Project LVATT\models\ggml-medium.bin)");

	if (ctx == nullptr) {
		fprintf(stderr, "error: failed to initialize whisper context\n");
		return 3;
	}

	std::string fname_inp = R"(C:\Programing Projects\C++\Project LVATT\build\x64-release\LVATT\Test.wav)";

	std::vector<float> pcmf32;               // mono-channel F32 PCM
	std::vector<std::vector<float>> pcmf32s; // stereo-channel F32 PCM

	if (!::read_wav(fname_inp, pcmf32, pcmf32s, false)) {
		fprintf(stderr, "error: failed to read WAV file '%s'\n", fname_inp.c_str());
		return -1;
	}

	// run the inference
	{
		whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

		wparams.print_realtime = false;
		wparams.language = "auto";
		wparams.n_threads = 14;

		wparams.new_segment_callback = whisper_print_segment_callback;
		wparams.new_segment_callback_user_data = nullptr;

		// print some info about the processing
		{
			fprintf(stderr, "%s: processing '%s' (%d samples, %.1f sec), %d threads, %d processors, lang = %s, task = %s, %stimestamps = %d ...\n",
				__func__, fname_inp.c_str(), int(pcmf32.size()), float(pcmf32.size()) / WHISPER_SAMPLE_RATE,
				wparams.n_threads, 1,
				wparams.language,
				wparams.translate ? "translate" : "transcribe",
				wparams.tdrz_enable ? "tdrz = 1, " : "",
				wparams.print_timestamps ? 1 : 0);

			fprintf(stderr, "\n");
		}

		if (whisper_full_parallel(ctx, wparams, pcmf32.data(), pcmf32.size(), 1) != 0) {
			fprintf(stderr, "failed to process audio\n");
			return 10;
		}
	}
	printf("\n");

	whisper_print_timings(ctx);
	whisper_free(ctx);

	return 0;
}
