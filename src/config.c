#include "./config.h"
#include "./draw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_SCREEN_SIZE 240

int WINDOW_WIDTH;
int WINDOW_HEIGHT;

static void writeDefaultConfig(const char *filename, int screenHeight) {
  FILE *file = fopen(filename, "w");
  fprintf(file, "# Size must be between 240 and your screen's height\n");
  fprintf(file, "WINDOW_SIZE=%d\n", (int)(screenHeight * 0.9));
  fclose(file);
}

void loadConfig(const char *filename, int screenWidth, int screenHeight) {
#if defined(ANDROID)
	WINDOW_WIDTH = max(screenWidth, screenHeight);
	WINDOW_HEIGHT = min(screenWidth, screenHeight);
#elif !defined(PC)
	WINDOW_WIDTH = screenWidth;
	WINDOW_HEIGHT = screenHeight;
#else
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    writeDefaultConfig(filename, screenHeight);
    WINDOW_WIDTH = (int)(screenHeight * 0.9);
    WINDOW_HEIGHT = (int)(screenHeight * 0.9);
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

  if (width >= MIN_SCREEN_SIZE && width <= screenHeight &&
    height >= MIN_SCREEN_SIZE && height <= screenHeight) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    validConfig = true;
  }

  if (!validConfig) {
    // Invalid config, create a new one with default values
    writeDefaultConfig(filename, screenHeight);
    WINDOW_WIDTH = (int)(screenHeight * 0.9);
    WINDOW_HEIGHT = (int)(screenHeight * 0.9);
  }
#endif
}