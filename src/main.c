#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./draw.h"
#include "./game.h"
#include "./math_custom.h"
#include "./input.h"

SDL_Window *window = NULL;
SDL_Surface *screen = NULL;
SDL_Renderer *renderer;
bool gameOver = false;

Uint64 now = 0;
Uint64 last = 0;
Uint64 gameStartTime = 0;
double deltaTime = 0;

int cubesLength = 0;
Cube cubes[1000];

static void prepareGame() {
  gameOver = false;
  for (i = 0; i < cubesLength; i++) {
    removeCube(cubes, i);
  }
  rearrangeCubesToTakeOutRemoved(cubes, &cubesLength, cubesLength);
  cubesLength = 0;
  gameStartTime = SDL_GetTicks();
  srand(time(NULL));
  gameInit(cubes, &cubesLength);
}

static void init() {
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow("Blockamok", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_GetWindowSurface(window);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  TTF_Init();
  controllerInit();
  widthMult = min((float)HEIGHT / WIDTH, 1);
  heightMult = min((float)WIDTH / HEIGHT, 1);
  cubeCollisionCompareX = 0.5 / heightMult;
  cubeCollisionCompareY = 0.5 / widthMult;
  prepareGame();
}

static void gameLoop() {
  if (!gameOver) {
    gameOver = gameFrame(deltaTime, cubes, &cubesLength);
  }
}

int main(int arg, char *argv[]) {
  init();

  while (!quit) {
    last = now;
    now = SDL_GetTicks();
    deltaTime = (double)((now - last)) / 12000;

    handlePlayerInput();
    gameLoop();

    draw(renderer);

    drawCubes(renderer, cubes, cubesLength);

    drawSpeedText(renderer);
    if (gameOver) {
      if (buttonPressed(INPUT_START)) {
        prepareGame();
      }
      drawGameOverText(renderer);
    }

    controllerAxis_leftStickX_last = controllerAxis_leftStickX;
    controllerAxis_leftStickY_last = controllerAxis_leftStickY;

    SDL_RenderPresent(renderer);
  }

  return 0;
}