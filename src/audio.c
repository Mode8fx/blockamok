#include <stdio.h>
#include "./audio.h"
#include "./audio/spaceranger_50k.h"
#include "./audio/mano_-_darkness_in_the_night.h"
#include "./audio/falling_people.h"
#include "./audio/falling_up.h"
#include "./audio/insanity.h"
#include "./input.h"

Sint8 audioIndex = 0;
#define NUM_SONGS 5

Mix_Music *bgm_1;
Mix_Music *bgm_2;
Mix_Music *bgm_3;
Mix_Music *bgm_4;
Mix_Music *bgm_5;

void initAudio() {
	bgm_1 = Mix_LoadMUS_RW(SDL_RWFromConstMem(spaceranger_50k_xm, spaceranger_50k_xm_len), 1);
	bgm_2 = Mix_LoadMUS_RW(SDL_RWFromConstMem(falling_up_mod, falling_up_mod_len), 1);
	bgm_3 = Mix_LoadMUS_RW(SDL_RWFromConstMem(falling_people_xm, falling_people_xm_len), 1);
	bgm_4 = Mix_LoadMUS_RW(SDL_RWFromConstMem(mano___darkness_in_the_night_xm, mano___darkness_in_the_night_xm_len), 1);
	bgm_5 = Mix_LoadMUS_RW(SDL_RWFromConstMem(insanity_s3m, insanity_s3m_len), 1);

	Mix_VolumeMusic(128);
	Mix_Volume(1, 128);
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
		audioIndex = (audioIndex - 1) % NUM_SONGS;
		if (audioIndex < 0) {
			audioIndex = NUM_SONGS - 1;
		}
		playMusicAtIndex(audioIndex);
	} else if (keyPressed(INPUT_R)) {
		audioIndex = (audioIndex + 1) % NUM_SONGS;
		playMusicAtIndex(audioIndex);
	}
}