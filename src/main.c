#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./draw.h"
#include "./text.h"
#include "./game.h"
#include "./math_custom.h"
#include "./input.h"
#include "./general.h"

SDL_Window *window = NULL;
SDL_Surface *screen = NULL;
SDL_Renderer *renderer;
int gameState = GAME_STATE_TITLE_SCREEN;

Uint64 now = 0;
Uint64 last = 0;
Uint64 gameStartTime = 0;
double deltaTime = 0;

int cubesLength = 0;
Cube cubes[1000];

double scoreVal;

static void prepareGame() {
  for (int i = 0; i < cubesLength; i++) {
    removeCube(cubes, i);
  }
  rearrangeCubesToTakeOutRemoved(cubes, &cubesLength, cubesLength);
  cubesLength = 0;
  gameStartTime = SDL_GetTicks();
  srand((Uint32)time(NULL));
  gameInit(cubes, &cubesLength);
}

static void init() {
  SDL_Init(SDL_INIT_EVERYTHING);
#if !defined(SDL1)
  SDL_GetCurrentDisplayMode(0, &DM);
#endif
  window = SDL_CreateWindow("Blockamok", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_GetWindowSurface(window);
  setScalingVals();
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  controllerInit();
  TTF_Init();
  initStaticMessages(renderer);
  widthMult = fminf((float)HEIGHT / WIDTH, 1);
  heightMult = fminf((float)WIDTH / HEIGHT, 1);
  cubeCollisionCompareX = 0.5f / heightMult;
  cubeCollisionCompareY = 0.5f / widthMult;
  prepareGame();
}

static void gameLoop() {
  if (gameState != GAME_STATE_GAME_OVER) {
    gameState = gameFrame((float)deltaTime, cubes, &cubesLength);
  }
}

int main(int arg, char *argv[]) {
  init();
  // Call once at the start for initial render
  gameFrame((float)deltaTime, cubes, &cubesLength);
  draw(renderer);

  while (!quit) {
    last = now;
    now = SDL_GetTicks();
    deltaTime = (double)((now - last)) / 12000;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }

    handlePlayerInput();

    switch (gameState) {
      case GAME_STATE_TITLE_SCREEN:
        if (keyPressed(INPUT_START)) {
          scoreVal = 0;
          gameState = GAME_STATE_PLAYING;
				} else if (keyPressed(INPUT_SELECT)) {
					gameState = GAME_STATE_CREDITS;
				}
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawTitleScreenText(renderer);
        break;
			case GAME_STATE_CREDITS:
        if (keyPressed(INPUT_SELECT)) {
          gameState = GAME_STATE_TITLE_SCREEN;
        }
				draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawCreditsText(renderer);
        break;
      case GAME_STATE_PLAYING:
        gameLoop();
        if (keyPressed(INPUT_START)) {
          gameState = GAME_STATE_PAUSED;
        }
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawScoreText(renderer);
        break;
      case GAME_STATE_PAUSED:
        if (keyPressed(INPUT_START)) {
          gameState = GAME_STATE_PLAYING;
        } else if (keyPressed(INPUT_SELECT)) {
          prepareGame();
          gameFrame((float)deltaTime, cubes, &cubesLength);
          gameState = GAME_STATE_TITLE_SCREEN;
        }
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawScoreText(renderer);
        drawPausedText(renderer);
        break;
      case GAME_STATE_GAME_OVER:
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawScoreText(renderer);
        drawGameOverText(renderer);
        if (keyPressed(INPUT_START)) {
          prepareGame();
          gameFrame((float)deltaTime, cubes, &cubesLength);
          gameState = GAME_STATE_TITLE_SCREEN;
        }
        break;
    }

    SDL_RenderPresent(renderer);
  }

  return 0;
}