#pragma once

#include <stdbool.h>

#define MIN_WINDOW_SIZE 240

extern void initFilePaths();
extern void writeSaveData();
extern void readSaveData(bool skipVisualSettings);
extern void loadConfig(int screenWidth, int screenHeight);