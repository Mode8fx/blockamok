#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./config.h"
#include "./draw.h"
#include "./game.h"
#include "./audio.h"
#include "./text.h"

int WINDOW_WIDTH;
int WINDOW_HEIGHT;
#define DEFAULT_SCREEN_SIZE (screenHeight * 0.75)

char rootDir[256];

char saveFile[256];
char configFile[256];

void initFilePaths() {
#if defined(VITA)
  snprintf(rootDir, sizeof(rootDir), "ux0:data/BlockamokRemix/");
#elif defined(WII)
  snprintf(rootDir, sizeof(rootDir), "sd:/apps/BlockamokRemix/");
#elif defined(GAMECUBE)
  snprintf(rootDir, sizeof(rootDir), "/BlockamokRemix/");
#elif defined(LINUX)
  snprintf(rootDir, sizeof(rootDir), "%s/.BlockamokRemix/", getenv("HOME"));
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
    fwrite(&highScoreVal, sizeof(highScoreVal), 1, file);
    fwrite(&OPTION_CUBE_FREQUENCY, sizeof(OPTION_CUBE_FREQUENCY), 1, file);
    fwrite(&OPTION_CUBE_SIZE, sizeof(OPTION_CUBE_SIZE), 1, file);
    fwrite(&OPTION_LIVES, sizeof(OPTION_LIVES), 1, file);
    fwrite(&OPTION_CONTROL_TYPE, sizeof(OPTION_CONTROL_TYPE), 1, file);
    fwrite(&OPTION_BACKGROUND_COLOR, sizeof(OPTION_BACKGROUND_COLOR), 1, file);
    fwrite(&OPTION_CUBE_COLOR, sizeof(OPTION_CUBE_COLOR), 1, file);
    fwrite(&OPTION_OVERLAY_COLOR, sizeof(OPTION_OVERLAY_COLOR), 1, file);
    fwrite(&OPTION_SPEEDOMETER, sizeof(OPTION_SPEEDOMETER), 1, file);
    fwrite(&OPTION_FULLSCREEN, sizeof(OPTION_FULLSCREEN), 1, file);
    fwrite(&OPTION_MUSIC, sizeof(OPTION_MUSIC), 1, file);
    fwrite(&OPTION_MUSIC_VOLUME, sizeof(OPTION_MUSIC_VOLUME), 1, file);
    fwrite(&OPTION_SFX_VOLUME, sizeof(OPTION_SFX_VOLUME), 1, file);

    Uint8 numBytesUsed = sizeof(highScoreVal)
      + sizeof(OPTION_CUBE_FREQUENCY) + sizeof(OPTION_CUBE_SIZE) + sizeof(OPTION_LIVES) + sizeof(OPTION_CONTROL_TYPE)
      + sizeof(OPTION_BACKGROUND_COLOR) + sizeof(OPTION_CUBE_COLOR) + sizeof(OPTION_OVERLAY_COLOR) + sizeof(OPTION_SPEEDOMETER) + sizeof(OPTION_FULLSCREEN)
      + sizeof(OPTION_MUSIC) + sizeof(OPTION_MUSIC_VOLUME) + sizeof(OPTION_SFX_VOLUME);
    Uint8 emptyBytesSize = 255 - numBytesUsed; // In case I want to add more to the save data in a future update
    if (emptyBytesSize > 0) {
      char *emptyBytes = (char *)calloc(emptyBytesSize, sizeof(char));
      fwrite(emptyBytes, emptyBytesSize, 1, file);
      free(emptyBytes); // Free the allocated memory
    }
    fclose(file);
  }
}

void readSaveData() {
  FILE *file = fopen(saveFile, "rb");
  if (file != NULL) {
    fread(&highScoreVal, sizeof(highScoreVal), 1, file);
    fread(&OPTION_CUBE_FREQUENCY, sizeof(OPTION_CUBE_FREQUENCY), 1, file);
    fread(&OPTION_CUBE_SIZE, sizeof(OPTION_CUBE_SIZE), 1, file);
    fread(&OPTION_LIVES, sizeof(OPTION_LIVES), 1, file);
    fread(&OPTION_CONTROL_TYPE, sizeof(OPTION_CONTROL_TYPE), 1, file);
    fread(&OPTION_BACKGROUND_COLOR, sizeof(OPTION_BACKGROUND_COLOR), 1, file);
    fread(&OPTION_CUBE_COLOR, sizeof(OPTION_CUBE_COLOR), 1, file);
    fread(&OPTION_OVERLAY_COLOR, sizeof(OPTION_OVERLAY_COLOR), 1, file);
    fread(&OPTION_SPEEDOMETER, sizeof(OPTION_SPEEDOMETER), 1, file);
    fread(&OPTION_FULLSCREEN, sizeof(OPTION_FULLSCREEN), 1, file);
    fread(&OPTION_MUSIC, sizeof(OPTION_MUSIC), 1, file);
    fread(&OPTION_MUSIC_VOLUME, sizeof(OPTION_MUSIC_VOLUME), 1, file);
    fread(&OPTION_SFX_VOLUME, sizeof(OPTION_SFX_VOLUME), 1, file);
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