#pragma once

#include <SDL.h>
#include <SDL_mixer.h>

extern Sint8 audioIndex;

extern void initAudio();
extern void playMusicAtIndex(Sint8 index);
extern void handleChangeSong();