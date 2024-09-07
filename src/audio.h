#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

#define SFX_ZOOM 0
#define SFX_CRASH 1

extern Sint8 audioIndex;

extern void initAudio();
extern void playMusicAtIndex(Sint8 index);
extern void handleChangeSong();
extern void playSFX(Sint8 index);
extern void cleanUpAudio();