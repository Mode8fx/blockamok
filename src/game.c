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

const int CUBE_AMOUNT = 600;

const unsigned long cubeMemSize = CUBE_POINTS_N * sizeof(Point);

const float BOUNDS_X = 12;
const float BOUNDS_Y = 12;
const float SPEED_INCREASE = 350;
const Sint8 SPEED_UP_MULT = 3;
const float MAX_SPEED = 1500;

const float CUBE_SIZE = 0.5;

float playerSpeed;

bool isAnalog;
Sint16 movementMagnitudeX;
Sint16 movementMagnitudeY;

static void addNewCube(Cube cubes[], int *cubesLength) {
  Point point = {
    randF(-BOUNDS_X, BOUNDS_X),
    randF(-BOUNDS_Y, BOUNDS_Y),
    MAX_DEPTH
  };
  cubes[(*cubesLength)++] = newCube(point, CUBE_SIZE);
}

static void addInitialCube(Cube cubes[], int *cubesLength) {
  Point point = {
    randF(-BOUNDS_X, BOUNDS_X),
    randF(-BOUNDS_Y, BOUNDS_Y),
    randF(0, MAX_DEPTH)
	};
  cubes[(*cubesLength)++] = newCube(point, CUBE_SIZE);
}

void gameInit(Cube cubes[], int *cubesLength) {
  playerSpeed = PLAYER_INITIAL_SPEED;
  while ((*cubesLength) < CUBE_AMOUNT) {
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
    for (p = 0; p < 20; p++) {
      cubes[i][p].x += BOUNDS_X * 2;
    }
  } else if (cubes[i][0].x > BOUNDS_X) {
    for (p = 0; p < 20; p++) {
      cubes[i][p].x -= BOUNDS_X * 2;
    }
  }
  if (cubes[i][0].y < -BOUNDS_Y) {
    for (p = 0; p < 20; p++) {
      cubes[i][p].y += BOUNDS_X * 2;
    }
  } else if (cubes[i][0].y > BOUNDS_Y) {
    for (p = 0; p < 20; p++) {
      cubes[i][p].y -= BOUNDS_Y * 2;
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

int gameFrame(float deltaTime, Cube cubes[], int *cubesLength) {
  while (*cubesLength < CUBE_AMOUNT) {
    addNewCube(cubes, cubesLength);
  }

  bool speedingUp = (keyHeld(INPUT_A) || keyHeld(INPUT_B));

  playerSpeed += deltaTime * SPEED_INCREASE * (speedingUp ? 3 : 1);

  float speed = playerSpeed * deltaTime;

  int cubesRemoved = 0;

  float xDiff = 0;
  float yDiff = 0;

  if (!isAnalog) {
    float turnSpeed = (BASE_TURN_SPEED_TYPE_A + playerSpeed / 50) * deltaTime;
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
    float turnSpeed = (BASE_TURN_SPEED_TYPE_B + playerSpeed / 50) * deltaTime;
    xDiff = turnSpeed * -movementMagnitudeX / 32767;
		yDiff = turnSpeed * -movementMagnitudeY / 32767;
  }

  float zSpeed = -speed;
  if (speedingUp) {
    zSpeed *= SPEED_UP_MULT;
  }
  if (zSpeed < -MAX_SPEED) {
    zSpeed = -MAX_SPEED;
  }

  for (int i = 0; i < (*cubesLength); i++) {
    if ((cubes[i][0].z + zSpeed) < 1.5) {
      removeCube(cubes, i);
      cubesRemoved += 1;
    } else {
      flipCubeIfOutOfBounds(cubes, i);
      for (int p = 0; p < 20; p++) {
        cubes[i][p].x += xDiff;
        cubes[i][p].y += yDiff;

        cubes[i][p].z += zSpeed;
      }

      float middleX = fabsf(cubes[i][0].x + (cubes[i][2].x - cubes[i][0].x) * 0.5f);
      float middleY = fabsf(cubes[i][0].y + (cubes[i][2].y - cubes[i][0].y) * 0.5f + 0.25f); // the +0.25f shifts the collision point downwards
      if (cubes[i][0].z < 2 && middleX < 0.5 && middleY < 0.5 && (SDL_GetTicks() - gameStartTime) > 1000) {
        playSFX(SFX_CRASH);
        if (scoreVal > highScoreVal) {
          highScoreVal = (int)scoreVal;
          newHighScore = true;
          refreshHighScoreText(renderer);
          writeSaveData();
        }
        return GAME_STATE_GAME_OVER;
      }
    }
  }

	scoreVal -= zSpeed; // zSpeed is negative

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

  // Left
  Point l1 = {.x = -half + c.x, .y = +half + c.y, .z = +half * 2 + c.z};
  Point l2 = {.x = -half + c.x, .y = -half + c.y, .z = +half * 2 + c.z};
  Point l3 = {.x = -half + c.x, .y = -half + c.y, .z = -half + c.z};
  Point l4 = {.x = -half + c.x, .y = +half + c.y, .z = -half + c.z};

  // Right
  Point r1 = {.x = +half + c.x, .y = +half + c.y, .z = +half * 2 + c.z};
  Point r2 = {.x = +half + c.x, .y = -half + c.y, .z = +half * 2 + c.z};
  Point r3 = {.x = +half + c.x, .y = -half + c.y, .z = -half + c.z};
  Point r4 = {.x = +half + c.x, .y = +half + c.y, .z = -half + c.z};

  // Front
  Point f1 = {.x = -half + c.x, .y = -half + c.y, .z = -half + c.z};
  Point f2 = {.x = +half + c.x, .y = -half + c.y, .z = -half + c.z};
  Point f3 = {.x = +half + c.x, .y = +half + c.y, .z = -half + c.z};
  Point f4 = {.x = -half + c.x, .y = +half + c.y, .z = -half + c.z};

  cubeAddr[0] = u1;
  cubeAddr[1] = u2;
  cubeAddr[2] = u3;
  cubeAddr[3] = u4;

  cubeAddr[4] = d1;
  cubeAddr[5] = d2;
  cubeAddr[6] = d3;
  cubeAddr[7] = d4;

  cubeAddr[8] = l1;
  cubeAddr[9] = l2;
  cubeAddr[10] = l3;
  cubeAddr[11] = l4;

  cubeAddr[12] = r1;
  cubeAddr[13] = r2;
  cubeAddr[14] = r3;
  cubeAddr[15] = r4;

  cubeAddr[16] = f1;
  cubeAddr[17] = f2;
  cubeAddr[18] = f3;
  cubeAddr[19] = f4;

  return cubeAddr;
}