#include <SDL.h>

#include "./game.h"

#define WIDTH 1000
#define HEIGHT 1000

#if WIDTH > HEIGHT
#define LARGER_ASPECT_RATIO  ((float)WIDTH / HEIGHT)
#define SMALLER_ASPECT_RATIO ((float)HEIGHT / WIDTH)
#define MINSCREEN HEIGHT
#else
#define LARGER_ASPECT_RATIO  ((float)HEIGHT / WIDTH)
#define SMALLER_ASPECT_RATIO ((float)WIDTH / HEIGHT)
#define MINSCREEN WIDTH
#endif

extern float sizeMult;
extern float widthMult, heightMult, cubeCollisionCompareX, cubeCollisionCompareY;

void draw(SDL_Renderer *renderer);

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

void drawCube(SDL_Renderer *renderer, Cube cube);

void drawTitleScreenText(SDL_Renderer *renderer);

void drawSpeedText(SDL_Renderer *renderer);

void drawGameOverText(SDL_Renderer *renderer);