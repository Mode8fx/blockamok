#pragma once

#define MIN_WINDOW_SIZE 240

extern void initFilePaths();
extern void writeSaveData();
extern void readSaveData(bool);
extern void loadConfig(int screenWidth, int screenHeight);