#include "./config.h"
#include "./draw.h"
#include "./game.h"
#include "./audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int WINDOW_WIDTH;
int WINDOW_HEIGHT;
#define DEFAULT_SCREEN_SIZE (screenHeight * 0.75)

char rootDir[256];

char saveFile[256];
char configFile[256];

void initFilePaths() {
#if defined(VITA)
  snprintf(rootDir, sizeof(rootDir), "ux0:data/Blockamok/");
#elif defined(WII)
  snprintf(rootDir, sizeof(rootDir), "sd:/apps/Blockamok/");
#elif defined(GAMECUBE)
  snprintf(rootDir, sizeof(rootDir), "/Blockamok/");
#elif defined(LINUX)
  snprintf(rootDir, sizeof(rootDir), "%s/.blockamok/", getenv("HOME"));
// else, rootDir remains empty
#endif
  snprintf(saveFile, sizeof(saveFile), "%s%s", rootDir, "save.bin");
  snprintf(configFile, sizeof(configFile), "%s%s", rootDir, "config.ini");
}

void writeSaveData() {
#if defined(LINUX) || defined(VITA)
  mkdir(rootDir, 0777);
#endif
  FILE *file = fopen(saveFile, "wb");
  if (file != NULL) {
    char emptyBytes[122] = {0}; // In case I want to add more to the save data in a future update
    fwrite(&highScoreVal, sizeof(highScoreVal), 1, file);
    fwrite(&isAnalog, sizeof(isAnalog), 1, file);
    fwrite(&audioIndex, sizeof(audioIndex), 1, file);
    fwrite(emptyBytes, sizeof(emptyBytes), 1, file);
    fclose(file);
  }
}

void readSaveData() {
  FILE *file = fopen(saveFile, "rb");
  if (file != NULL) {
    fread(&highScoreVal, sizeof(highScoreVal), 1, file);
    fread(&isAnalog, sizeof(isAnalog), 1, file);
    fread(&audioIndex, sizeof(audioIndex), 1, file);
    audioIndex %= NUM_SONGS;
    fclose(file);
  } else {
    writeSaveData();
  }
}

static void writeDefaultConfig(int screenHeight) {
  FILE *file = fopen(configFile, "w");
  fprintf(file, "# Size must be between 240 and your screen's height\n");
  fprintf(file, "WINDOW_SIZE=%d\n", (int)DEFAULT_SCREEN_SIZE);
  fclose(file);
}

void loadConfig(int screenWidth, int screenHeight) {
#if defined(ANDROID)
	WINDOW_WIDTH = (int)fmax(screenWidth, screenHeight);
	WINDOW_HEIGHT = (int)fmin(screenWidth, screenHeight);
#elif !defined(PC)
	WINDOW_WIDTH = screenWidth;
	WINDOW_HEIGHT = screenHeight;
#else
  FILE *file = fopen(configFile, "r");
  if (file == NULL) {
    writeDefaultConfig(screenHeight);
    WINDOW_WIDTH = (int)DEFAULT_SCREEN_SIZE;
    WINDOW_HEIGHT = (int)DEFAULT_SCREEN_SIZE;
    return;
  }

  char line[256];
  int width = 0, height = 0;
  bool validConfig = false;

  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "WINDOW_SIZE=", 12) == 0) {
      width = atoi(line + 12);
      height = atoi(line + 12);
      break;
    }
  }

  fclose(file);

  if (width >= MIN_WINDOW_SIZE && width <= screenHeight &&
    height >= MIN_WINDOW_SIZE && height <= screenHeight) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    validConfig = true;
  }

  if (!validConfig) {
    // Invalid config, create a new one with default values
    writeDefaultConfig(screenHeight);
    WINDOW_WIDTH = (int)DEFAULT_SCREEN_SIZE;
    WINDOW_HEIGHT = (int)DEFAULT_SCREEN_SIZE;
  }
#endif
}