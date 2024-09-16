#pragma once

#include <SDL.h>
#include <stdbool.h>

#define CUBE_FACE_POINTS_N 4
#define CUBE_FACE_N 5
#define CUBE_POINTS_N 20  // 5 * 4

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
extern float scoreVal;
extern int highScoreVal;
extern bool newHighScore;
extern bool showCursor;

#define MAX_SPEED 1500.0f
#define MAX_SPEED_INT (int)MAX_SPEED
extern float playerSpeed;
extern bool speedingUp;
#define SPEED_UP_MULT 2.2f
#define TRUE_MAX_SPEED (MAX_SPEED * SPEED_UP_MULT)
#define TRUE_MAX_SPEED_INT (int)TRUE_MAX_SPEED

extern Sint16 movementMagnitudeX;
extern Sint16 movementMagnitudeY;

extern Sint16 cubeAmount;
extern float cubeSize;
extern float cubeSizeHalf;
extern float cubeSizeLimit;
extern Sint8 numLives;

extern bool debugMode;

// Weaker systems need to handle fewer cubes, and smaller bounds balance this out
#define DEFAULT_CUBE_BOUNDS 12.0f
extern float cubeBounds;
#define CUBE_LIMIT_MULT (cubeBounds / DEFAULT_CUBE_BOUNDS)
#define CUBE_LIMIT_MAX (Sint16)(800 * CUBE_LIMIT_MULT)

#define CUBE_REMOVAL_DEPTH 1.5f

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

typedef struct {
  Point points[CUBE_POINTS_N];
} Cube;

int gameFrame(Uint32 deltaTime, Cube cubes[]);

void gameInit(Cube cubes[]);
void resetCube(Cube cubes[], int i);
extern void prepareGame();

Cube newCube(float s, Sint16 i);