#pragma once

#include <SDL.h>
#include <stdbool.h>

#define CUBE_FACE_POINTS_N 4
#define CUBE_FACE_N 5
#define CUBE_POINTS_N 20  // 5 * 4
#define FRONT_I 16        // 4 * 4

#define GAME_STATE_TITLE_SCREEN 0
#define GAME_STATE_INSTRUCTIONS 1
#define GAME_STATE_CREDITS 2
#define GAME_STATE_PLAYING 3
#define GAME_STATE_PAUSED 4
#define GAME_STATE_GAME_OVER 5
#define GAME_STATE_STARTED 6

extern Uint64 gameStartTime;
extern double scoreVal;
extern int highScoreVal;
extern bool newHighScore;

extern bool isAnalog;
extern Sint16 movementMagnitudeX;
extern Sint16 movementMagnitudeY;

typedef struct {
  float z;
} Player;

typedef struct Point {
  float x;
  float y;
  float z;
} Point;

//typedef struct {
//} Block;

typedef Point* Cube;

int gameFrame(float deltaTime, Cube cubes[], int* cubesLength);

void gameInit(Cube cubes[], int* cubesLength);
void removeCube(Cube cubes[], int i);
void rearrangeCubesToTakeOutRemoved(Cube cubes[], int *cubesLength, int removedN);

Cube newCube(Point c, float s);