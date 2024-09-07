#pragma once

#include <SDL.h>

#include "./game.h"

extern SDL_DisplayMode DM;
extern SDL_Renderer *renderer;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
#define GAME_WIDTH WINDOW_HEIGHT // square
#define GAME_HEIGHT WINDOW_HEIGHT

#define MAX_DEPTH 150
extern float HALF_FOV_ANGLE_RADIANS;

void setScalingVals();

void draw(SDL_Renderer *renderer);

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

void drawCube(SDL_Renderer *renderer, Cube cube);