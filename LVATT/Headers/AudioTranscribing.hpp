#include <whisper.h>

#include "Common.hpp"

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

	for (int i = s0; i < n_segments; i++) {

		t0 = whisper_full_get_segment_t0(ctx, i);
		t1 = whisper_full_get_segment_t1(ctx, i);
		printf("[%s --> %s]  ", to_timestamp(t0).c_str(), to_timestamp(t1).c_str());
		const char* text = whisper_full_get_segment_text(ctx, i);

		printf("%s%s", speaker.c_str(), text);

		fflush(stdout);
	}
}

int TranscribeAudio(ArrayWrapper<float> audio)
{
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
		__func__, "abc", int(audio.size), float(audio.size) / WHISPER_SAMPLE_RATE,
		wparams.n_threads, 2,
		wparams.language,
		wparams.translate ? "translate" : "transcribe",
		wparams.tdrz_enable ? "tdrz = 1, " : "",
		wparams.print_timestamps ? 1 : 0);

	if (whisper_full_parallel(ctx, wparams, audio.data, audio.size, 2) != 0)
	{
		fprintf(stderr, "failed to process audio\n");
		return 10;
	}
	whisper_free(ctx);
}