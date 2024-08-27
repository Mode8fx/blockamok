#include <stdio.h>
#include "./audio.h"
#include "./audio/spaceranger_50k.h"
#include "./audio/mano_-_darkness_in_the_night.h"
#include "./input.h"

Sint8 audioIndex = 0;

Mix_Music *bgm_1;
Mix_Music *bgm_2;

void initAudio() {
	bgm_1 = Mix_LoadMUS_RW(SDL_RWFromConstMem(spaceranger_50k_xm, spaceranger_50k_xm_len), 1);
	bgm_2 = Mix_LoadMUS_RW(SDL_RWFromConstMem(mano___darkness_in_the_night_xm, mano___darkness_in_the_night_xm_len), 1);

	Mix_VolumeMusic(128);
	Mix_Volume(1, 128); // Ensure channel 1 is the correct channel you want to set volume for
}

void playMusicAtIndex(Sint8 index) {
	switch (index) {
		case 0:
			Mix_PlayMusic(bgm_1, -1);
			break;
		case 1:
			Mix_PlayMusic(bgm_2, -1);
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