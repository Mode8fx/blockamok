#include "./game.h"

#include <SDL.h>
#include <math.h>

#include "./math_custom.h"
#include "./draw.h"
#include "./input.h"

const float PLAYER_INITIAL_SPEED = 100;
const float BASE_TURN_SPEED = 30;

const int CUBE_AMOUNT = 600;

const unsigned long cubeMemSize = CUBE_POINTS_N * sizeof(Point);

const float BOUNDS_X = 12;
const float BOUNDS_Y = 12;
const float SPEED_INCREASE = 350;
const Sint8 SPEED_UP_MULT = 3;

const float CUBE_SIZE = 0.5;

float playerSpeed;
bool speedingUp;

Point point;
Cube cube, cube1, cube2;
int fullI;
int i, p;

float speed, turnSpeed;
int cubesRemoved;
float xDiff, yDiff;
float zSpeed;
float middleX, middleY;

float half;
Cube cubeAddr;
Point u1, u2, u3, u4, d1, d2, d3, d4, l1, l2, l3, l4, r1, r2, r3, r4, f1, f2, f3, f4;

static void addNewCube(Cube cubes[], int *cubesLength) {
  point.x = randF(-BOUNDS_X, BOUNDS_X);
  point.y = randF(-BOUNDS_Y, BOUNDS_Y);
  point.z = MAX_DEPTH;
  cube = newCube(point, CUBE_SIZE);
  cubes[(*cubesLength)++] = cube;
}

static void addInitialCube(Cube cubes[], int *cubesLength) {
  point.x = randF(-BOUNDS_X, BOUNDS_X);
  point.y = randF(-BOUNDS_Y, BOUNDS_Y);
  point.z = randF(0, MAX_DEPTH);
  cube = newCube(point, CUBE_SIZE);
  cubes[(*cubesLength)++] = cube;
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

  fullI = 0;
  for (i = 0; i < (*cubesLength); i++) {
    if (cubes[i] != NULL) {
      cubes[fullI++] = cubes[i];
    }
  }
}

static void flipCubeIfOutOfBounds(Cube cubes[], int i) {
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
  cube1 = *((Cube *)a);
  cube2 = *((Cube *)b);
  if (cube1[0].z == cube2[0].z) {
    return (cube1[0].x < cube2[0].x) - (cube1[0].x > cube2[0].x);
  }
  return (cube1[0].z < cube2[0].z) - (cube1[0].z > cube2[0].z);
}

int gameFrame(float deltaTime, Cube cubes[], int *cubesLength) {
  while (*cubesLength < CUBE_AMOUNT) {
    addNewCube(cubes, cubesLength);
  }

  speedingUp = (keyHeld(INPUT_A));

  playerSpeed += deltaTime * (SPEED_INCREASE + (speedingUp * SPEED_UP_MULT));

  speed = playerSpeed * deltaTime;
  turnSpeed = (BASE_TURN_SPEED + playerSpeed / 50) * deltaTime;

  cubesRemoved = 0;

  xDiff = 0;
  yDiff = 0;
  if (keyHeld(INPUT_UP)) {
    yDiff = +turnSpeed;
  }
  if (keyHeld(INPUT_DOWN)) {
    yDiff = -turnSpeed;
  }
  if (keyHeld(INPUT_LEFT)) {
    xDiff = +turnSpeed;
  }
  if (keyHeld(INPUT_RIGHT)) {
    xDiff = -turnSpeed;
  }
  zSpeed = -speed;
  if (speedingUp) {
    zSpeed *= SPEED_UP_MULT;
  }

  for (i = 0; i < (*cubesLength); i++) {
    if ((cubes[i][0].z + zSpeed) < 1.5) {
      removeCube(cubes, i);
      cubesRemoved += 1;
    } else {
      flipCubeIfOutOfBounds(cubes, i);
      for (p = 0; p < 20; p++) {
        cubes[i][p].x += xDiff;
        cubes[i][p].y += yDiff;

        cubes[i][p].z += zSpeed;
      }

      middleX = fabs(cubes[i][0].x + (cubes[i][2].x - cubes[i][0].x) / 2);
      middleY = fabs(cubes[i][0].y + (cubes[i][2].y - cubes[i][0].y) / 2);
      //if (cubes[i][0].z < 2 && middleX < cubeCollisionCompareX && middleY < cubeCollisionCompareY && (SDL_GetTicks() - gameStartTime) > 1000) {
      if (cubes[i][0].z < 2 && middleX < 0.5 && middleY < 0.5 && (SDL_GetTicks() - gameStartTime) > 1000) {
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
  half = s / 2.0;

  cubeAddr = malloc(cubeMemSize);

  // Up
  u1.x = -half + c.x;
  u1.y = -half + c.y;
  u1.z = +half * 2 + c.z;
  u2.x = +half + c.x;
  u2.y = -half + c.y;
  u2.z = +half * 2 + c.z;
  u3.x = +half + c.x;
  u3.y = -half + c.y;
  u3.z = -half + c.z;
  u4.x = -half + c.x;
  u4.y = -half + c.y;
  u4.z = -half + c.z;

  // Down
  d1.x = -half + c.x;
  d1.y = +half + c.y;
  d1.z = +half * 2 + c.z;
  d2.x = +half + c.x;
  d2.y = +half + c.y;
  d2.z = +half * 2 + c.z;
  d3.x = +half + c.x;
  d3.y = +half + c.y;
  d3.z = -half + c.z;
  d4.x = -half + c.x;
  d4.y = +half + c.y;
  d4.z = -half + c.z;

  // Left
  l1.x = -half + c.x;
  l1.y = +half + c.y;
  l1.z = +half * 2 + c.z;
  l2.x = -half + c.x;
  l2.y = -half + c.y;
  l2.z = +half * 2 + c.z;
  l3.x = -half + c.x;
  l3.y = -half + c.y;
  l3.z = -half + c.z;
  l4.x = -half + c.x;
  l4.y = +half + c.y;
  l4.z = -half + c.z;

  // Right
  r1.x = +half + c.x;
  r1.y = +half + c.y;
  r1.z = +half * 2 + c.z;
  r2.x = +half + c.x;
  r2.y = -half + c.y;
  r2.z = +half * 2 + c.z;
  r3.x = +half + c.x;
  r3.y = -half + c.y;
  r3.z = -half + c.z;
  r4.x = +half + c.x;
  r4.y = +half + c.y;
  r4.z = -half + c.z;

  // Front
  f1.x = -half + c.x;
  f1.y = -half + c.y;
  f1.z = -half + c.z;
  f2.x = +half + c.x;
  f2.y = -half + c.y;
  f2.z = -half + c.z;
  f3.x = +half + c.x;
  f3.y = +half + c.y;
  f3.z = -half + c.z;
  f4.x = -half + c.x;
  f4.y = +half + c.y;
  f4.z = -half + c.z;

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