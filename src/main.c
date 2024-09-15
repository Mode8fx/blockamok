#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "./config.h"
#include "./draw.h"
#include "./text.h"
#include "./audio.h"
#include "./game.h"
#include "./math_custom.h"
#include "./input.h"
#include "./general.h"
#include "./game_init.h"

SDL_Window *window = NULL;
SDL_Surface *screen = NULL;
SDL_Renderer *renderer;
int gameState = GAME_STATE_STARTED;
bool drawOverlayOnThisFrame = true;

Uint32 now = 0;
Uint32 last = 0;
Uint32 gameStart = 0;
Uint32 invinceStart = 0;
Uint32 deltaTime = 0;
Uint32 credits_startTime = 0;

int cubesLength = 0;
Cube cubes[1000];

float scoreVal;
#define DEFAULT_HIGH_SCORE 1000
int highScoreVal = DEFAULT_HIGH_SCORE;
bool newHighScore = false;
bool showCursor = true;

Sint8 highScoreResetIndex = 0;
#define HIGH_SCORE_RESET_SEQUENCE_LENGTH 8
static const int highScoreResetSequence[HIGH_SCORE_RESET_SEQUENCE_LENGTH] = {
  INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_UP, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT
};

Sint8 invincibilityResetIndex = 0;
#define INVINCIBILITY_SEQUENCE_LENGTH 11
static const int invincibilitySequence[INVINCIBILITY_SEQUENCE_LENGTH] = {
  INPUT_UP, INPUT_UP, INPUT_UP, INPUT_DOWN, INPUT_DOWN, INPUT_DOWN, INPUT_LEFT, INPUT_RIGHT, INPUT_LEFT, INPUT_RIGHT, INPUT_LEFT
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
  if (abs(WINDOW_WIDTH - WINDOW_HEIGHT) <= 0.06 * fmax(WINDOW_WIDTH, WINDOW_HEIGHT)) {
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
  drawOverlayOnThisFrame = true;
#endif
}

void prepareGame() {
  for (int i = 0; i < cubesLength; i++) {
    removeCube(cubes, i);
  }
  rearrangeCubesToTakeOutRemoved(cubes, &cubesLength, cubesLength);
  cubesLength = 0;
  gameInit(cubes, &cubesLength);
  // Call once for initial render
  gameFrame(deltaTime, cubes, &cubesLength);
}

static void init() {
#if defined(PSP)
  window = SDL_CreateWindow("Blockamok Remix", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
#elif defined(PC)
  window = SDL_CreateWindow("Blockamok Remix", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
#else
  window = SDL_CreateWindow("Blockamok Remix", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
#endif
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  screen = SDL_GetWindowSurface(window);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  controllerInit();
  TTF_Init();
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
  initAudio();
  startingTick = SDL_GetTicks();
}

static inline void gameLoop() {
  if (gameState != GAME_STATE_GAME_OVER) {
    gameState = gameFrame(deltaTime, cubes, &cubesLength);
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
      gameState = GAME_STATE_OPTIONS_MAIN;
      openPage(renderer, &optionPage_Main, false);
    }
	} else if (pressedKeys != 0) {
    highScoreResetIndex = 0;
	}
}

static void handleInvincibility() {
  if (keyPressed(invincibilitySequence[invincibilityResetIndex])) {
    invincibilityResetIndex++;
    if (invincibilityResetIndex >= INVINCIBILITY_SEQUENCE_LENGTH) {
      if (!isInvincible) {
        isInvincible = true;
        playSFX(SFX_DING_A);
      } else {
        isInvincible = false;
        playSFX(SFX_DING_B);
      }
      invincibilityResetIndex = 0;
    }
  } else if (pressedKeys != 0) {
    invincibilityResetIndex = 0;
  }
}

static void handleFullscreenToggle() {
#if defined(PC)
  if (keyPressed(INPUT_RS)) {
    OPTION_FULLSCREEN = !OPTION_FULLSCREEN;
    optionCallback_Fullscreen(window, &optionPage_Visual);
  }
#endif
}

int main(int arg, char *argv[]) {
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
  SDL_GetCurrentDisplayMode(0, &DM);
  initFilePaths();
  loadConfig(DM.w, DM.h);
  init();
  setScalingVals();
  initStaticMessages(renderer);
  readSaveData();
  optionCallback_All();
  playMusicAtIndex(OPTION_MUSIC);
  if (OPTION_FULLSCREEN) {
    optionCallback_Fullscreen(window, &optionPage_Visual);
  }
  srand((Uint32)time(NULL));
  prepareGame();
  draw(renderer);

  while (!quit) {
    last = now;
    now = SDL_GetTicks();
    deltaTime = now - last;

    SDL_RenderSetViewport(renderer, &gameViewport);

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
    handleFullscreenToggle();

    switch (gameState) {
      case GAME_STATE_STARTED:
        drawEssentials(renderer, cubes, cubesLength);
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
          numLives = OPTION_LIVES + 1;
          playSFX(SFX_ZOOM);
          gameStart = SDL_GetTicks();
          invinceStart = gameStart;
          gameState = GAME_STATE_PLAYING;
        } else if (keyPressed(INPUT_SELECT)) {
          openPage(renderer, &optionPage_Main, true);
          gameState = GAME_STATE_OPTIONS_MAIN;
        }
        drawEssentials(renderer, cubes, cubesLength);
        drawTitleScreenText(renderer, true);
        break;

      case GAME_STATE_OPTIONS_MAIN:
        if (keyPressed(INPUT_A) || keyPressed(INPUT_START)) {
          switch (optionPage_Main.index) {
            case 4:
              credits_paused = false;
              credits_startTime = SDL_GetTicks();
              break;
            case 6:
              forceIndexReset = true;
              initStaticMessages(renderer);
              optionCallback_All();
              playSFX(SFX_THUNK);
              forceIndexReset = false;
              break;
            default:
              break;
          }
        }
				drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Main, true);
				break;

      case GAME_STATE_OPTIONS_GAME:
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Game, true);
        break;

      case GAME_STATE_OPTIONS_VISUAL:
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Visual, true);
        break;

      case GAME_STATE_OPTIONS_AUDIO:
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Audio, true);
        break;

      case GAME_STATE_INSTRUCTIONS:
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Empty, false);
        drawInstructionsText(renderer);
        break;

			case GAME_STATE_CREDITS:
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Empty, false);
        drawCreditsText(renderer, now);
        break;

      case GAME_STATE_RESET_HIGH_SCORE:
        handleResetHighScore();
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Empty, false);
        drawResetHighScoreText(renderer);
        break;

      case GAME_STATE_QUIT:
        if (keyPressed(INPUT_A) || keyPressed(INPUT_START)) {
          quit = true;
          writeSaveData();
        }
        drawEssentials(renderer, cubes, cubesLength);
        handlePage(renderer, window, &optionPage_Empty, false);
        drawQuitText(renderer);
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
        drawEssentials(renderer, cubes, cubesLength);
        drawGameText(renderer);
        drawCursor(renderer);
        break;

      case GAME_STATE_PAUSED:
        handleInvincibility();
        if (keyPressed(INPUT_START)) {
          gameState = GAME_STATE_PLAYING;
        } else if (keyPressed(INPUT_SELECT)) {
          prepareGame();
          gameState = GAME_STATE_TITLE_SCREEN;
        }
        drawEssentials(renderer, cubes, cubesLength);
        drawGameText(renderer);
        drawPausedText(renderer);
        break;

      case GAME_STATE_GAME_OVER:
        if (keyPressed(INPUT_START)) {
          newHighScore = false;
          prepareGame();
          gameState = GAME_STATE_TITLE_SCREEN;
        }
        drawEssentials(renderer, cubes, cubesLength);
        drawGameText(renderer);
        drawGameOverText(renderer);
        break;
    }

    SDL_RenderSetViewport(renderer, NULL);
    //if (drawOverlayOnThisFrame) {
      SDL_SetRenderDrawColor(renderer, overlayColor.r, overlayColor.g, overlayColor.b, 255);
      SDL_RenderFillRect(renderer, &leftBar);
      SDL_RenderFillRect(renderer, &rightBar);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderFillRect(renderer, &leftBorder);
      SDL_RenderFillRect(renderer, &rightBorder);
      //drawOverlayOnThisFrame = false;
    //}
    SDL_RenderPresent(renderer);
  }

  cleanUpText();
  cleanUpMenu();
  TTF_Quit();
  cleanUpAudio();
  Mix_CloseAudio();
  Mix_Quit();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  systemSpecificClose();

  return 0;
}