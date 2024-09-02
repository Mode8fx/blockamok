#include <SDL.h>
#include <SDL_mixer.h>
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
#include "./game_init.h"

SDL_Window *window = NULL;
SDL_Surface *screen = NULL;
SDL_Renderer *renderer;
int gameState = GAME_STATE_STARTED;

Uint64 now = 0;
Uint64 last = 0;
Uint64 gameStartTime = 0;
double deltaTime = 0;
Uint64 credits_startTime = 0;

int cubesLength = 0;
Cube cubes[1000];

double scoreVal;

static void prepareGame() {
  for (int i = 0; i < cubesLength; i++) {
    removeCube(cubes, i);
  }
  rearrangeCubesToTakeOutRemoved(cubes, &cubesLength, cubesLength);
  cubesLength = 0;
  srand((Uint32)time(NULL));
  gameInit(cubes, &cubesLength);
}

static void init() {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
#if !defined(SDL1)
  SDL_GetCurrentDisplayMode(0, &DM);
#endif
  window = SDL_CreateWindow("Blockamok", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_GetWindowSurface(window);
  setScalingVals();
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  controllerInit();
  TTF_Init();
  initStaticMessages(renderer);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  initAudio();
  playMusicAtIndex(0);
  startingTick = SDL_GetTicks();
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

  int gameOffsetX = (SCREEN_WIDTH - GAME_WIDTH) / 2;
  SDL_Rect gameViewport = { gameOffsetX, 0, GAME_WIDTH, GAME_HEIGHT };
  SDL_Rect leftBar = { 0, 0, gameOffsetX, SCREEN_HEIGHT };
  SDL_Rect rightBar = { gameOffsetX + GAME_WIDTH, 0, gameOffsetX, SCREEN_HEIGHT };

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
    handleChangeSong();

    SDL_RenderSetViewport(renderer, &gameViewport);

    switch (gameState) {
      case GAME_STATE_STARTED:
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawTitleScreenText(renderer, false);
        fadeInFromBlack(renderer);
        if (now - startingTick > INIT_FADE_LENGTH) {
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
          gameState = GAME_STATE_TITLE_SCREEN;
        }
        break;
      case GAME_STATE_TITLE_SCREEN:
        if (keyPressed(INPUT_START)) {
          scoreVal = 0;
          gameStartTime = SDL_GetTicks();
          gameState = GAME_STATE_PLAYING;
        } else if (keyPressed(INPUT_X)) {
          gameState = GAME_STATE_INSTRUCTIONS;
				} else if (keyPressed(INPUT_Y)) {
          credits_paused = false;
					credits_startTime = SDL_GetTicks();
					gameState = GAME_STATE_CREDITS;
				} else if (keyPressed(INPUT_SELECT)) {
          quit = true;
        }
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawTitleScreenText(renderer, true);
        break;

      case GAME_STATE_INSTRUCTIONS:
        if (keyPressed(INPUT_LEFT) || keyPressed(INPUT_RIGHT)) {
					isAnalog = !isAnalog;
        } else if (keyPressed(INPUT_B) || keyPressed(INPUT_START) || keyPressed(INPUT_SELECT)) {
          gameState = GAME_STATE_TITLE_SCREEN;
        }
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawInstructionsText(renderer);
        break;

			case GAME_STATE_CREDITS:
        if (keyPressed(INPUT_B) || keyPressed(INPUT_START) || keyPressed(INPUT_SELECT)) {
          gameState = GAME_STATE_TITLE_SCREEN;
        }
				draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawCreditsText(renderer, now);
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

    SDL_RenderSetViewport(renderer, NULL);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &leftBar);
    SDL_RenderFillRect(renderer, &rightBar);
    SDL_RenderPresent(renderer);
  }

  return 0;
}