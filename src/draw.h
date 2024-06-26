#include <SDL.h>

#include "./game.h"

#if !defined(SDL1)
SDL_DisplayMode DM;
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

extern float LARGET_ASPECT_RATIO;
extern float SMALLER_ASPECT_RATIO;
extern int MINSCREEN;

extern float MAX_DEPTH;
extern float HALF_FOV_ANGLE_RADIANS;

extern float sizeMult;
extern float widthMult, heightMult, cubeCollisionCompareX, cubeCollisionCompareY;

void setScalingVals();

void draw(SDL_Renderer *renderer);

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

void drawCube(SDL_Renderer *renderer, Cube cube);

void drawTitleScreenText(SDL_Renderer *renderer);

void drawSpeedText(SDL_Renderer *renderer);

void drawGameOverText(SDL_Renderer *renderer);