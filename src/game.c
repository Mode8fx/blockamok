#include <SDL.h>
#include <math.h>
#include <stdlib.h>

#include "./game.h"
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
float cubeSizeHalf = 0.25f;
float cubeSizeLimit = 0.5f;
Sint8 numLives = 3;

const Sint16 SPEED_INCREASE = 350;

float playerSpeed;
bool speedingUp;

Sint16 movementMagnitudeX;
Sint16 movementMagnitudeY;

bool debugMode = false;

#if defined(GAMECUBE)
float cubeBounds = 8.3f;
#elif defined(WII)
float cubeBounds = 10.5f;
#elif defined(WII_U)
float cubeBounds = 8.3f; // TODO: test this
#elif defined(SWITCH)
float cubeBounds = 8.5f; // TODO: test this
#elif defined(PSP) || defined(THREEDS)
float cubeBounds = 2.5f; // these versions are basically a proof-of-concept
#elif defined(VITA)
float cubeBounds = 8.1f;
#else
float cubeBounds = DEFAULT_CUBE_BOUNDS;
#endif

void gameInit(Cube cubes[]) {
  playerSpeed = PLAYER_INITIAL_SPEED;
  for (Sint16 i = 0; i < cubeAmount; i++) {
    cubes[i] = newCube(cubeSize, i);
  }
}

static inline float randF(float min, float max) {
  return min + (float)rand() / ((float)RAND_MAX / (max - min));
}

void resetCube(Cube cubes[], int i) {
  float relX = randF(-cubeBounds, cubeBounds) - cubes[i].points[0].x;
  float relY = randF(-cubeBounds, cubeBounds) - cubes[i].points[0].y;
  for (int p = 0; p < 8; p++) {
    cubes[i].points[p].x += relX;
    cubes[i].points[p].y += relY;
    cubes[i].points[p].z += MAX_DEPTH;
  }
  cubes[i].points[8] = cubes[i].points[4];
  cubes[i].points[9] = cubes[i].points[0];
  cubes[i].points[10] = cubes[i].points[3];
  cubes[i].points[11] = cubes[i].points[7];
  cubes[i].points[12] = cubes[i].points[5];
  cubes[i].points[13] = cubes[i].points[1];
  cubes[i].points[14] = cubes[i].points[2];
  cubes[i].points[15] = cubes[i].points[6];
  cubes[i].points[16] = cubes[i].points[3];
  cubes[i].points[17] = cubes[i].points[2];
  cubes[i].points[18] = cubes[i].points[6];
  cubes[i].points[19] = cubes[i].points[7];
}

static void flipCubeIfOutOfBounds(Cube cubes[], int i) {
  int p;
  if (cubes[i].points[0].x < -cubeBounds) {
    float cubeBoundsMult = cubeBounds * 2;
    for (p = 0; p < 20; p++) {
      cubes[i].points[p].x += cubeBoundsMult;
    }
  } else if (cubes[i].points[0].x > cubeBounds) {
    float cubeBoundsMult = cubeBounds * 2;
    for (p = 0; p < 20; p++) {
      cubes[i].points[p].x -= cubeBoundsMult;
    }
  }
  if (cubes[i].points[0].y < -cubeBounds) {
    float cubeBoundsMult = cubeBounds * 2;
    for (p = 0; p < 20; p++) {
      cubes[i].points[p].y += cubeBoundsMult;
    }
  } else if (cubes[i].points[0].y > cubeBounds) {
    float cubeBoundsMult = cubeBounds * 2;
    for (p = 0; p < 20; p++) {
      cubes[i].points[p].y -= cubeBoundsMult;
    }
  }
}

static int compareSize(const void *a, const void *b) {
  Cube cube1 = *((Cube *)a);
  Cube cube2 = *((Cube *)b);
  if (cube1.points[0].z == cube2.points[0].z) {
    return (cube1.points[0].x < cube2.points[0].x) - (cube1.points[0].x > cube2.points[0].x);
  }
  return (cube1.points[0].z < cube2.points[0].z) - (cube1.points[0].z > cube2.points[0].z);
}

int gameFrame(Uint32 deltaTime, Cube cubes[]) {
  speedingUp = (keyHeld(INPUT_A) || keyHeld(INPUT_B));

  float deltaTimeDiv = (float)deltaTime / 12000;

  playerSpeed += deltaTimeDiv * SPEED_INCREASE * (speedingUp ? 3 : 1);
  if (playerSpeed > MAX_SPEED) {
    playerSpeed = MAX_SPEED;
  }

  float speed = playerSpeed * deltaTimeDiv;

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
    float turnSpeed = (BASE_TURN_SPEED_TYPE_B + playerSpeed / 50) * deltaTimeDiv / 32767;
    xDiff = turnSpeed * -movementMagnitudeX;
    yDiff = turnSpeed * -movementMagnitudeY;
  }

  float zSpeed = speed;
  if (speedingUp) {
    zSpeed *= SPEED_UP_MULT;
  }

  for (int i = 0; i < cubeAmount; i++) {
    bool shouldResetCube = (cubes[i].points[0].z - zSpeed) < CUBE_REMOVAL_DEPTH;

    flipCubeIfOutOfBounds(cubes, i);
    for (int p = 0; p < 8; p++) {
      cubes[i].points[p].x += xDiff;
      cubes[i].points[p].y += yDiff;

      cubes[i].points[p].z -= zSpeed;
    }
    cubes[i].points[8] = cubes[i].points[4];
    cubes[i].points[9] = cubes[i].points[0];
    cubes[i].points[10] = cubes[i].points[3];
    cubes[i].points[11] = cubes[i].points[7];
    cubes[i].points[12] = cubes[i].points[5];
    cubes[i].points[13] = cubes[i].points[1];
    cubes[i].points[14] = cubes[i].points[2];
    cubes[i].points[15] = cubes[i].points[6];
    cubes[i].points[16] = cubes[i].points[3];
    cubes[i].points[17] = cubes[i].points[2];
    cubes[i].points[18] = cubes[i].points[6];
    cubes[i].points[19] = cubes[i].points[7];

    if (cubes[i].points[0].z < 2) {
      // left edge of cube on cursor is cubeSize/2, right edge is -cubeSize/2
      float middleX = fabsf(cubes[i].points[0].x + (cubes[i].points[2].x - cubes[i].points[0].x) * 0.5f);
      // top edge of cube on cursor is 0, bottom edge is cubeSize
      float middleY = fabsf(cubes[i].points[0].y + (cubes[i].points[2].y - cubes[i].points[0].y) * 0.5f + cubeSizeHalf); // the +cubeSizeHalf shifts the collision point downwards
      if (middleX < cubeSizeLimit && middleY < cubeSizeLimit && (SDL_GetTicks() - invinceStart) > INVINCE_TIME && !debugMode) {
        // gameFrame() can be called when preparing game, so check for that first
        if (gameState != GAME_STATE_PLAYING) {
          if (shouldResetCube) {
            resetCube(cubes, i);
          }
          qsort(cubes, cubeAmount, sizeof(Cube), compareSize);
          return gameState;
        }
        playSFX(SFX_THUNK);
        if (--numLives > 0) {
          playerSpeed = (float)fmin(playerSpeed, MAX_SPEED) - (MAX_SPEED * 0.3f);
          if (playerSpeed < PLAYER_INITIAL_SPEED) {
            playerSpeed = PLAYER_INITIAL_SPEED;
          }
          invinceStart = SDL_GetTicks();
        } else {
          if (shouldResetCube) {
            float depthDiff = CUBE_REMOVAL_DEPTH - cubes[i].points[0].z;
            for (int p = 0; p < 20; p++) {
              cubes[i].points[p].z += depthDiff;
            }
          }
          scoreVal += zSpeed;
          if (scoreVal > highScoreVal) {
            highScoreVal = (int)scoreVal;
            newHighScore = true;
            writeSaveData();
          }
          qsort(cubes, cubeAmount, sizeof(Cube), compareSize);
          return GAME_STATE_GAME_OVER;
        }
      }
    }
    if (shouldResetCube) {
      resetCube(cubes, i);
    }
  }

	scoreVal += zSpeed;
  qsort(cubes, cubeAmount, sizeof(Cube), compareSize);

  return GAME_STATE_PLAYING;
}

Cube newCube(float s, Sint16 i) {
  Point c = {
    randF(-cubeBounds, cubeBounds),
    randF(-cubeBounds, cubeBounds),
    MAX_DEPTH / cubeAmount * i
  };

  // Try not to spawn any blocks too close to the player
  if (c.z <= 5) {
    for (int i = 0; i < 5; i++) {
      if (fabsf(c.x) < 2 || fabsf(c.y) < 2) {
        c.x = randF(-cubeBounds, cubeBounds);
        c.y = randF(-cubeBounds, cubeBounds);
      } else {
        break;
      }
    }
  }

  float half = s * 0.5f;

  Cube cube;

  // Up
  cube.points[0] = (Point){ .x = -half + c.x, .y = -half + c.y, .z = +s + c.z };
  cube.points[1] = (Point){ .x = +half + c.x, .y = -half + c.y, .z = +s + c.z };
  cube.points[2] = (Point){ .x = +half + c.x, .y = -half + c.y, .z = -half + c.z };
  cube.points[3] = (Point){ .x = -half + c.x, .y = -half + c.y, .z = -half + c.z };

  // Down
  cube.points[4] = (Point){ .x = -half + c.x, .y = +half + c.y, .z = +s + c.z };
  cube.points[5] = (Point){ .x = +half + c.x, .y = +half + c.y, .z = +s + c.z };
  cube.points[6] = (Point){ .x = +half + c.x, .y = +half + c.y, .z = -half + c.z };
  cube.points[7] = (Point){ .x = -half + c.x, .y = +half + c.y, .z = -half + c.z };

  // Left
  cube.points[8] = cube.points[4];
  cube.points[9] = cube.points[0];
  cube.points[10] = cube.points[3];
  cube.points[11] = cube.points[7];

  // Right
  cube.points[12] = cube.points[5];
  cube.points[13] = cube.points[1];
  cube.points[14] = cube.points[2];
  cube.points[15] = cube.points[6];

  // Front
  cube.points[16] = cube.points[3];
  cube.points[17] = cube.points[2];
  cube.points[18] = cube.points[6];
  cube.points[19] = cube.points[7];

  return cube;
}