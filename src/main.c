#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./draw.h"
#include "./text.h"
#include "./audio.h"
#include "./game.h"
#include "./math_custom.h"
#include "./input.h"
#include "./general.h"
#include "./config.h"
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
#define DEFAULT_HIGH_SCORE 1000
int highScoreVal = DEFAULT_HIGH_SCORE;
bool newHighScore = false;
bool showCursor = true;

Sint8 highScoreResetIndex = 0;
#define HIGH_SCORE_RESET_SEQUENCE_LENGTH 8
static const int highScoreResetSequence[HIGH_SCORE_RESET_SEQUENCE_LENGTH] = {
  INPUT_UP, INPUT_B, INPUT_DOWN, INPUT_B, INPUT_LEFT, INPUT_B, INPUT_RIGHT, INPUT_B
};

static void handleWindowResize(SDL_Event *event) {
#if defined(PC)
  WINDOW_WIDTH = event->window.data1;
  WINDOW_HEIGHT = event->window.data2;
  bool needsReset = false;
  if (WINDOW_WIDTH < MIN_WINDOW_SIZE) {
    WINDOW_WIDTH = MIN_WINDOW_SIZE;
    needsReset = true;
  } if (WINDOW_HEIGHT < MIN_WINDOW_SIZE) {
    WINDOW_HEIGHT = MIN_WINDOW_SIZE;
    needsReset = true;
  }
  if (abs(WINDOW_WIDTH - WINDOW_HEIGHT) <= 0.06 * max(WINDOW_WIDTH, WINDOW_HEIGHT)) {
    if (WINDOW_WIDTH > WINDOW_HEIGHT) {
      WINDOW_WIDTH = WINDOW_HEIGHT;
    }
    else {
      WINDOW_HEIGHT = WINDOW_WIDTH;
    }
    needsReset = true;
  }
  if (WINDOW_HEIGHT > WINDOW_WIDTH) {
    WINDOW_HEIGHT = WINDOW_WIDTH;
    needsReset = true;
  }
  if (needsReset) {
    SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
  }
  setScalingVals();
  initStaticMessages(renderer);
#endif
}

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
  window = SDL_CreateWindow("Blockamok", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_GetWindowSurface(window);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  controllerInit();
  TTF_Init();
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  initAudio();
  playMusicAtIndex(audioIndex);
  startingTick = SDL_GetTicks();
  prepareGame();
}

static inline void gameLoop() {
  if (gameState != GAME_STATE_GAME_OVER) {
    gameState = gameFrame((float)deltaTime, cubes, &cubesLength);
  }
}

static void handleResetHighScore() {
  if (keyPressed(highScoreResetSequence[highScoreResetIndex])) {
    highScoreResetIndex++;
    if (highScoreResetIndex >= HIGH_SCORE_RESET_SEQUENCE_LENGTH) {
      highScoreVal = DEFAULT_HIGH_SCORE;
      refreshHighScoreText(renderer);
      writeSaveData();
			playSFX(SFX_THUNK);
      highScoreResetIndex = 0;
    }
	} else if (pressedKeys != 0) {
    highScoreResetIndex = 0;
	}
}

int main(int arg, char *argv[]) {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
  SDL_GetCurrentDisplayMode(0, &DM);
  initFilePaths();
  loadConfig(DM.w, DM.h);
  readSaveData();
  init();
  setScalingVals();
  initStaticMessages(renderer);
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
#if defined(PC)
      else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          handleWindowResize(&event);
        }
      }
#endif
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
        handleResetHighScore();
        if (keyPressed(INPUT_START)) {
          scoreVal = 0;
          playSFX(SFX_ZOOM);
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
          writeSaveData();
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
        } else if (keyPressed(INPUT_X) || keyPressed(INPUT_Y)) {
          if (!showCursor) {
            showCursor = true;
            playSFX(SFX_DING_A);
          } else {
            showCursor = false;
            playSFX(SFX_DING_B);
          }
        }
        draw(renderer);
        drawCubes(renderer, cubes, cubesLength);
        drawScoreText(renderer);
        drawCursor(renderer);
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
          newHighScore = false;
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

  cleanUpText();
  cleanUpAudio();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  systemSpecificClose();

  return 0;
}