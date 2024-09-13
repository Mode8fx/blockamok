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
#define GAME_STATE_OPTIONS_MAIN 7
#define GAME_STATE_OPTIONS_GAME 8
#define GAME_STATE_OPTIONS_VISUAL 9
#define GAME_STATE_OPTIONS_AUDIO 10
#define GAME_STATE_RESET_HIGH_SCORE 11
#define GAME_STATE_QUIT 12

extern int gameState;
extern Uint32 now;
extern Uint32 gameStart;
extern Uint32 invinceStart;
extern double scoreVal;
extern int highScoreVal;
extern bool newHighScore;
extern bool showCursor;

extern Sint16 movementMagnitudeX;
extern Sint16 movementMagnitudeY;

extern Sint16 cubeAmount;
extern float cubeSize;
extern Sint8 numLives;

#define INVINCE_TIME 1000
#define INVINCE_BLINK_TIME 111

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

typedef Point *Cube;

int gameFrame(Uint32 deltaTime, Cube cubes[], int *cubesLength);

void gameInit(Cube cubes[], int *cubesLength);
void removeCube(Cube cubes[], int i);
void rearrangeCubesToTakeOutRemoved(Cube cubes[], int *cubesLength, int removedN);
extern void prepareGame();

Cube newCube(Point c, float s);