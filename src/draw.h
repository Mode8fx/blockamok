#pragma once

#include <SDL.h>

#include "./game.h"

extern SDL_DisplayMode DM;
extern SDL_Renderer *renderer;

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
#define GAME_WIDTH WINDOW_HEIGHT // square
#define GAME_HEIGHT WINDOW_HEIGHT
extern SDL_Rect leftBar;
extern SDL_Rect rightBar;
extern SDL_Rect leftBorder;
extern SDL_Rect rightBorder;
extern int gameOffsetX;

extern SDL_Texture *backgroundTexture;
extern SDL_Color backgroundColor;
extern SDL_Color cubeColorFront;
extern SDL_Color cubeColorSide;
extern SDL_Color overlayColor;

#if defined(PSP)
#define SDL_RenderSetViewport(renderer, rect);
#endif
#define MAX_DEPTH 150.0f
extern float HALF_FOV_ANGLE_RADIANS;
extern float HALF_FOV_ANGLE_RADIANS_TAN;

void setScalingVals();

extern inline void draw(SDL_Renderer *renderer);

extern void drawEssentials(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

extern void saveBackgroundAsTexture(SDL_Renderer *renderer);