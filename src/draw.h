#include <SDL.h>

#include "./game.h"

#define WIDTH 1920
#define HEIGHT 1080

extern float sizeMult;
extern float widthMult, heightMult, cubeCollisionCompareX, cubeCollisionCompareY;
extern int MINSCREEN;

void draw(SDL_Renderer *renderer);

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength);

void drawCube(SDL_Renderer *renderer, Cube cube);

void drawSpeedText(SDL_Renderer *renderer);

void drawGameOverText(SDL_Renderer *renderer);