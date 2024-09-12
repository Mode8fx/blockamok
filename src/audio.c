#include <stdio.h>

#include "./audio.h"
#include "./audio/spaceranger_50k.h"
#include "./audio/mano_-_darkness_in_the_night.h"
#include "./audio/falling_people.h"
#include "./audio/falling_up.h"
#include "./audio/insanity.h"
#include "./audio/ding2_short.h"
#include "./audio/ding2_short_reversed.h"
#include "./audio/thunk.h"
#include "./audio/zoom3_short.h"
#include "./input.h"
#include "./config.h"
#include "./text.h"

Mix_Music *bgm_1;
Mix_Music *bgm_2;
Mix_Music *bgm_3;
Mix_Music *bgm_4;
Mix_Music *bgm_5;
Mix_Chunk *sfx_zoom;
Mix_Chunk *sfx_thunk;
Mix_Chunk *sfx_ding_a;
Mix_Chunk *sfx_ding_b;

void initAudio() {
	Mix_VolumeMusic(128);
	bgm_1 = Mix_LoadMUS_RW(SDL_RWFromConstMem(spaceranger_50k_xm, spaceranger_50k_xm_len), 1);
	bgm_2 = Mix_LoadMUS_RW(SDL_RWFromConstMem(falling_up_mod, falling_up_mod_len), 1);
	bgm_3 = Mix_LoadMUS_RW(SDL_RWFromConstMem(falling_people_xm, falling_people_xm_len), 1);
	bgm_4 = Mix_LoadMUS_RW(SDL_RWFromConstMem(mano___darkness_in_the_night_xm, mano___darkness_in_the_night_xm_len), 1);
	bgm_5 = Mix_LoadMUS_RW(SDL_RWFromConstMem(insanity_s3m, insanity_s3m_len), 1);

	Mix_Volume(1, 128);
	sfx_zoom = Mix_LoadWAV_RW(SDL_RWFromConstMem(zoom3_short_wav, zoom3_short_wav_len), 1);
	sfx_thunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(thunk_wav, thunk_wav_len), 1);
	sfx_ding_a = Mix_LoadWAV_RW(SDL_RWFromConstMem(ding2_short_wav, ding2_short_wav_len), 1);
	Mix_VolumeChunk(sfx_ding_a, 64);
	sfx_ding_b = Mix_LoadWAV_RW(SDL_RWFromConstMem(ding2_short_reversed_wav, ding2_short_reversed_wav_len), 1);
	Mix_VolumeChunk(sfx_ding_b, 64);
}

void playMusicAtIndex(Sint8 index) {
	switch (index) {
		case 0:
			Mix_PlayMusic(bgm_1, -1);
			break;
		case 1:
			Mix_PlayMusic(bgm_2, -1);
			break;
		case 2:
			Mix_PlayMusic(bgm_3, -1);
			break;
		case 3:
			Mix_PlayMusic(bgm_4, -1);
			break;
		case 4:
			Mix_PlayMusic(bgm_5, -1);
			break;
		default:
			break;
	}
}

void handleChangeSong() {
	if (keyPressed(INPUT_L)) {
		OPTION_MUSIC = (OPTION_MUSIC - 1) % NUM_SONGS;
		if (OPTION_MUSIC < 0) {
			OPTION_MUSIC = NUM_SONGS - 1;
		}
		writeSaveData();
		playMusicAtIndex(OPTION_MUSIC);
	} else if (keyPressed(INPUT_R)) {
		OPTION_MUSIC = (OPTION_MUSIC + 1) % NUM_SONGS;
		writeSaveData();
		playMusicAtIndex(OPTION_MUSIC);
	}
}

void playSFX(Sint8 index) {
	switch (index) {
		case 0:
			Mix_PlayChannel(-1, sfx_zoom, 0);
			break;
		case 1:
			Mix_PlayChannel(-1, sfx_thunk, 0);
			break;
		case 2:
			Mix_PlayChannel(-1, sfx_ding_a, 0);
			break;
		case 3:
			Mix_PlayChannel(-1, sfx_ding_b, 0);
			break;
		default:
			break;
	}
}

static void destroyMusic(Mix_Music *music) {
	if (music != NULL) {
		Mix_FreeMusic(music);
		music = NULL;
	}
}

static void destroyChunk(Mix_Chunk *chunk) {
	if (chunk != NULL) {
		Mix_FreeChunk(chunk);
		chunk = NULL;
	}
}

void cleanUpAudio() {
	destroyMusic(bgm_1);
	destroyMusic(bgm_2);
	destroyMusic(bgm_3);
	destroyMusic(bgm_4);
	destroyMusic(bgm_5);
	destroyChunk(sfx_zoom);
	destroyChunk(sfx_thunk);
	destroyChunk(sfx_ding_a);
	destroyChunk(sfx_ding_b);
}