#include <SDL.h>
#include <math.h>
#include <stdlib.h>

#include "./game.h"
#include "./math_custom.h"
#include "./draw.h"
#include "./input.h"
#include "./audio.h"
#include "./config.h"
#include "./text.h"

const float PLAYER_INITIAL_SPEED = 100;
const float BASE_TURN_SPEED_TYPE_A = 30; // effectively 42.43 when diagonal
const float BASE_TURN_SPEED_TYPE_B = 36.21f;

Sint16 cubeAmount = 600;
float cubeSize = 0.5f;
float cubeSizeLimit = 0.5f;
Sint8 numLives = 3;

const unsigned long cubeMemSize = CUBE_POINTS_N * sizeof(Point);

const float BOUNDS_X = 12;
const float BOUNDS_Y = 12;
const float SPEED_INCREASE = 350;

float playerSpeed;
bool speedingUp;

Sint16 movementMagnitudeX;
Sint16 movementMagnitudeY;

static void addNewCube(Cube cubes[], int *cubesLength) {
  Point point = {
    randF(-BOUNDS_X, BOUNDS_X),
    randF(-BOUNDS_Y, BOUNDS_Y),
    MAX_DEPTH
  };
  cubes[(*cubesLength)++] = newCube(point, cubeSize);
}

static void addInitialCube(Cube cubes[], int *cubesLength) {
  Point point = {
    randF(-BOUNDS_X, BOUNDS_X),
    randF(-BOUNDS_Y, BOUNDS_Y),
    randF(0, MAX_DEPTH)
	};
  cubes[(*cubesLength)++] = newCube(point, cubeSize);
}

void gameInit(Cube cubes[], int *cubesLength) {
  playerSpeed = PLAYER_INITIAL_SPEED;
  while ((*cubesLength) < cubeAmount) {
    addInitialCube(cubes, cubesLength);
  }
}

void removeCube(Cube cubes[], int i) {
  free(cubes[i]);
  cubes[i] = NULL;
}

void rearrangeCubesToTakeOutRemoved(Cube cubes[], int *cubesLength, int removedN) {
  if (removedN == 0) {
    return;
  }

  int fullI = 0;
  for (int i = 0; i < (*cubesLength); i++) {
    if (cubes[i] != NULL) {
      cubes[fullI++] = cubes[i];
    }
  }
}

static void flipCubeIfOutOfBounds(Cube cubes[], int i) {
  int p;
  if (cubes[i][0].x < -BOUNDS_X) {
    float BOUNDS_X_MULT = BOUNDS_X * 2;
    for (p = 0; p < 20; p++) {
      cubes[i][p].x += BOUNDS_X_MULT;
    }
  } else if (cubes[i][0].x > BOUNDS_X) {
    float BOUNDS_X_MULT = BOUNDS_X * 2;
    for (p = 0; p < 20; p++) {
      cubes[i][p].x -= BOUNDS_X_MULT;
    }
  }
  if (cubes[i][0].y < -BOUNDS_Y) {
    float BOUNDS_Y_MULT = BOUNDS_Y * 2;
    for (p = 0; p < 20; p++) {
      cubes[i][p].y += BOUNDS_Y_MULT;
    }
  } else if (cubes[i][0].y > BOUNDS_Y) {
    float BOUNDS_Y_MULT = BOUNDS_Y * 2;
    for (p = 0; p < 20; p++) {
      cubes[i][p].y -= BOUNDS_Y_MULT;
    }
  }
}

static int compareSize(const void *a, const void *b) {
  Cube cube1 = *((Cube *)a);
  Cube cube2 = *((Cube *)b);
  if (cube1[0].z == cube2[0].z) {
    return (cube1[0].x < cube2[0].x) - (cube1[0].x > cube2[0].x);
  }
  return (cube1[0].z < cube2[0].z) - (cube1[0].z > cube2[0].z);
}

int gameFrame(Uint32 deltaTime, Cube cubes[], int *cubesLength) {
  while (*cubesLength < cubeAmount) {
    addNewCube(cubes, cubesLength);
  }

  speedingUp = (keyHeld(INPUT_A) || keyHeld(INPUT_B));

  float deltaTimeDiv = (float)deltaTime / 12000;

  playerSpeed += deltaTimeDiv * SPEED_INCREASE * (speedingUp ? 3 : 1);
  if (playerSpeed > MAX_SPEED) {
    playerSpeed = MAX_SPEED;
  }

  float speed = playerSpeed * deltaTimeDiv;

  int cubesRemoved = 0;

  float xDiff = 0;
  float yDiff = 0;

  if (OPTION_CONTROL_TYPE == 0) {
    float turnSpeed = (BASE_TURN_SPEED_TYPE_A + playerSpeed / 50) * deltaTimeDiv;
    if (dirHeld_Up()) {
      yDiff = +turnSpeed;
    }
    if (dirHeld_Down()) {
      yDiff = -turnSpeed;
    }
    if (dirHeld_Left()) {
      xDiff = +turnSpeed;
    }
    if (dirHeld_Right()) {
      xDiff = -turnSpeed;
    }
  }
  else {
    float turnSpeed = (BASE_TURN_SPEED_TYPE_B + playerSpeed / 50) * deltaTimeDiv;
    xDiff = turnSpeed * -movementMagnitudeX / 32767;
    yDiff = turnSpeed * -movementMagnitudeY / 32767;
  }

  float zSpeed = speed;
  if (speedingUp) {
    zSpeed *= SPEED_UP_MULT;
  }

  for (int i = 0; i < (*cubesLength); i++) {
    bool shouldRemove = (cubes[i][0].z - zSpeed) < 1.5;

    flipCubeIfOutOfBounds(cubes, i);
    for (int p = 0; p < 20; p++) {
      cubes[i][p].x += xDiff;
      cubes[i][p].y += yDiff;

      cubes[i][p].z -= zSpeed;
    }

    float middleX = fabsf(cubes[i][0].x + (cubes[i][2].x - cubes[i][0].x) * 0.5f);
    float middleY = fabsf(cubes[i][0].y + (cubes[i][2].y - cubes[i][0].y) * 0.5f + 0.25f); // the +0.25f shifts the collision point downwards
    bool closeToCube = cubes[i][0].z < 2;
    if (shouldRemove) {
      removeCube(cubes, i);
      cubesRemoved += 1;
    }
    if (closeToCube && middleX < cubeSizeLimit && middleY < cubeSizeLimit && (SDL_GetTicks() - invinceStart) > INVINCE_TIME) {
      playSFX(SFX_THUNK);
      if (--numLives > 0) {
        playerSpeed = (float)fmin(playerSpeed, MAX_SPEED) - (MAX_SPEED * 0.3f);
        if (playerSpeed < PLAYER_INITIAL_SPEED) {
          playerSpeed = PLAYER_INITIAL_SPEED;
        }
        invinceStart = SDL_GetTicks();
      } else {
        if (scoreVal > highScoreVal) {
          highScoreVal = (int)scoreVal;
          newHighScore = true;
          refreshHighScoreText(renderer);
          writeSaveData();
        }
        rearrangeCubesToTakeOutRemoved(cubes, cubesLength, cubesRemoved);
        *cubesLength -= cubesRemoved;
        qsort(cubes, *cubesLength, sizeof(Cube*), compareSize);
        return GAME_STATE_GAME_OVER;
      }
    }
  }

	scoreVal += zSpeed;

  rearrangeCubesToTakeOutRemoved(cubes, cubesLength, cubesRemoved);
  *cubesLength -= cubesRemoved;
  qsort(cubes, *cubesLength, sizeof(Cube *), compareSize);

  return GAME_STATE_PLAYING;
}

Cube newCube(Point c, float s) {
  float half = s * 0.5f;

  Cube cubeAddr = malloc(cubeMemSize);

  // Up
  Point u1 = {.x = -half + c.x, .y = -half + c.y, .z = +half * 2 + c.z};
  Point u2 = {.x = +half + c.x, .y = -half + c.y, .z = +half * 2 + c.z};
  Point u3 = {.x = +half + c.x, .y = -half + c.y, .z = -half + c.z};
  Point u4 = {.x = -half + c.x, .y = -half + c.y, .z = -half + c.z};

  // Down
  Point d1 = {.x = -half + c.x, .y = +half + c.y, .z = +half * 2 + c.z};
  Point d2 = {.x = +half + c.x, .y = +half + c.y, .z = +half * 2 + c.z};
  Point d3 = {.x = +half + c.x, .y = +half + c.y, .z = -half + c.z};
  Point d4 = {.x = -half + c.x, .y = +half + c.y, .z = -half + c.z};

  // Up
  cubeAddr[0] = u1;
  cubeAddr[1] = u2;
  cubeAddr[2] = u3;
  cubeAddr[3] = u4;

  // Down
  cubeAddr[4] = d1;
  cubeAddr[5] = d2;
  cubeAddr[6] = d3;
  cubeAddr[7] = d4;

  // Left
  cubeAddr[8] = d1;
  cubeAddr[9] = u1;
  cubeAddr[10] = u4;
  cubeAddr[11] = d4;

  // Right
  cubeAddr[12] = d2;
  cubeAddr[13] = u2;
  cubeAddr[14] = u3;
  cubeAddr[15] = d3;

  // Front
  cubeAddr[16] = u4;
  cubeAddr[17] = u3;
  cubeAddr[18] = d3;
  cubeAddr[19] = d4;

  return cubeAddr;
}