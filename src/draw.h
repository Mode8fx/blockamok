#pragma once

#include <SDL.h>

#include "./game.h"

#if !defined(SDL1)
extern SDL_DisplayMode DM;
#endif

#if defined(ANDROID)
#define WIDTH  max(DM.w, DM.h)
#define HEIGHT min(DM.w, DM.h)
#elif defined(SDL1)
#define WIDTH  1000
#define HEIGHT 1000
#elif !defined(PC)
#define WIDTH  DM.w
#define HEIGHT DM.h
#else
#define WIDTH 1000
#define HEIGHT 1000
#endif

#define MAX_DEPTH 150
extern float HALF_FOV_ANGLE_RADIANS;

void setScalingVals();

void draw(SDL_Renderer *renderer);

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

void drawCube(SDL_Renderer *renderer, Cube cube);