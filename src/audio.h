#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

extern Sint8 audioIndex;
#define NUM_SONGS 2

extern void initAudio();
extern void playMusicAtIndex(Sint8 index);
extern void handleChangeSong();