#pragma once
#include "Common.hpp"

#include <whisper.h>

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

/// <summary>
/// Used to print transcribed text
/// </summary>
/// <param name="ctx">- whisper context</param>
/// <param name="">- whisper state</param>
/// <param name="n_new">- amount of new segments(?)</param>
/// <param name="user_data">- user data past in when creating wparams</param>
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

/// <summary>
/// transcribes audio stream
/// </summary>
/// <param name="audio">- audio data</param>
/// <param name="modelPath">- path to model used for transcribing</param>
/// <returns>will return none 0 number if failed</returns>
int TranscribeAudio(const ArrayWrapper<float>& audio, const std::string& modelPath)
{
	struct whisper_context* ctx = whisper_init_from_file(modelPath.c_str());

	if (ctx == nullptr) {
		fprintf(stderr, "error: failed to initialize whisper context\n");
		return 3;
	}

	// run the inference
	{
		whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);

		wparams.print_realtime = false;
		wparams.language = "auto";
		wparams.translate = true;
		wparams.n_threads = std::thread::hardware_concurrency();

		wparams.new_segment_callback = whisper_print_segment_callback;
		wparams.new_segment_callback_user_data = nullptr;

		// print some info about the processing
		{
			printf("processing audio (%d samples, %.1f sec), %d threads, %d processors, lang = %s, task = transcribe%s, %stimestamps = %d ...\n",
				int(audio.size), float(audio.size) / WHISPER_SAMPLE_RATE,
				wparams.n_threads, 1,
				wparams.language,
				wparams.translate ? " & translate" : "",
				wparams.tdrz_enable ? "tdrz = 1, " : "",
				wparams.print_timestamps ? 1 : 0);

			fprintf(stderr, "\n");
		}

		if (whisper_full_parallel(ctx, wparams, audio.data, audio.size, 1) != 0) {
			fprintf(stderr, "failed to process audio\n");
			return 10;
		}
	}
	printf("\n");

	whisper_print_timings(ctx);
	whisper_free(ctx);

	return 0;
}
