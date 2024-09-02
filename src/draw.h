#pragma once

#include <SDL.h>

#include "./game.h"

#if !defined(SDL1)
extern SDL_DisplayMode DM;
#endif

#if defined(ANDROID)
#define SCREEN_WIDTH  max(DM.w, DM.h)
#define SCREEN_HEIGHT min(DM.w, DM.h)
#elif defined(SDL1)
#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 1000
#elif !defined(PC)
#define SCREEN_WIDTH  DM.w
#define SCREEN_HEIGHT DM.h
#else
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#endif

#define GAME_WIDTH SCREEN_HEIGHT // square
#define GAME_HEIGHT SCREEN_HEIGHT

#define MAX_DEPTH 150
extern float HALF_FOV_ANGLE_RADIANS;

void setScalingVals();

void draw(SDL_Renderer *renderer);

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

void drawCube(SDL_Renderer *renderer, Cube cube);