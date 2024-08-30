#pragma once
#include <SDL.h>

extern Uint32 startingTick;
#define INIT_FADE_LENGTH 2000

extern void fadeInFromBlack(SDL_Renderer *renderer);