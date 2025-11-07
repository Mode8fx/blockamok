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
bool usedDebugMode = false;

#if defined(GAMECUBE)
float cubeBoundsBase = 9.2f; // 60 FPS
#elif defined(WII)
float cubeBoundsBase = DEFAULT_CUBE_BOUNDS; // 60 FPS
#elif defined(WII_U)
float cubeBoundsBase = 7.5f; // 31-60 FPS (about half the difficulty settings hit 60 or close)
#elif defined(SWITCH)
float cubeBoundsBase = 8.7f; // 60 FPS
#elif defined(THREEDS)
float cubeBoundsBase = 4.8f; // 40-60 FPS on New 3DS, 12-20 FPS on Old 3DS
#elif defined(PSP)
float cubeBoundsBase = 3.5f; // 23-48 FPS without overclock, 46-58 FPS with 333 MHz (full) overclock
#elif defined(VITA)
float cubeBoundsBase = 8.5f; // 45-60 FPS without overclock, 60 FPS with 444 MHz (partial) overclock
#elif defined(ANDROID)
float cubeBoundsBase = 10.0f;
#else
float cubeBoundsBase = DEFAULT_CUBE_BOUNDS;
#endif

float cubeBounds = 0;

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
  static const int pointMapping[12] = {4, 0, 3, 7, 5, 1, 2, 6, 3, 2, 6, 7};
  
  Point *points = cubes[i].points;
  float relX = randF(-cubeBounds, cubeBounds) - points[0].x;
  float relY = randF(-cubeBounds, cubeBounds) - points[0].y;
  
  for (int p = 0; p < 8; p++) {
    points[p].x += relX;
    points[p].y += relY;
    points[p].z += MAX_DEPTH;
  }
  
  // Copy duplicate points using lookup table
  for (int p = 0; p < 12; p++) {
    points[8 + p] = points[pointMapping[p]];
  }
}

static void flipCubeIfOutOfBounds(Cube cubes[], int i) {
  Point *points = cubes[i].points;
  float x0 = points[0].x;
  float y0 = points[0].y;
  
  if (x0 < -cubeBounds || x0 > cubeBounds) {
    float xOffset = (x0 < -cubeBounds) ? (cubeBounds * 2.0f) : (-cubeBounds * 2.0f);
    for (int p = 0; p < 20; p++) {
      points[p].x += xOffset;
    }
  }
  
  if (y0 < -cubeBounds || y0 > cubeBounds) {
    float yOffset = (y0 < -cubeBounds) ? (cubeBounds * 2.0f) : (-cubeBounds * 2.0f);
    for (int p = 0; p < 20; p++) {
      points[p].y += yOffset;
    }
  }
}

static int compareSize(const void *a, const void *b) {
  const Cube *cube1 = (const Cube *)a;
  const Cube *cube2 = (const Cube *)b;
  
  float z1 = cube1->points[0].z;
  float z2 = cube2->points[0].z;
  
  if (z1 == z2) {
    float x1 = cube1->points[0].x;
    float x2 = cube2->points[0].x;
    return (x1 < x2) - (x1 > x2);
  }
  return (z1 < z2) - (z1 > z2);
}

int gameFrame(Uint32 deltaTime, Cube cubes[]) {
  static const int pointMapping[12] = { 4, 0, 3, 7, 5, 1, 2, 6, 3, 2, 6, 7 };

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
  } else {
    float turnSpeed = (BASE_TURN_SPEED_TYPE_B + playerSpeed / 50) * deltaTimeDiv / 32767;
    xDiff = turnSpeed * -movementMagnitudeX;
    yDiff = turnSpeed * -movementMagnitudeY;
  }

  float zSpeed = speed;
  if (speedingUp) {
    zSpeed *= SPEED_UP_MULT;
  }

  bool isInvincible = (SDL_GetTicks() - invinceStart) <= INVINCE_TIME || debugMode;

  for (int i = 0; i < cubeAmount; i++) {
    Point *points = cubes[i].points;
    bool shouldResetCube = (points[0].z - zSpeed) < CUBE_REMOVAL_DEPTH;

    flipCubeIfOutOfBounds(cubes, i);

    // Update first 8 unique points
    for (int p = 0; p < 8; p++) {
      points[p].x += xDiff;
      points[p].y += yDiff;
      points[p].z -= zSpeed;
    }

    // Copy duplicate points using lookup table
    for (int p = 0; p < 12; p++) {
      points[8 + p] = points[pointMapping[p]];
    }

    if (points[0].z < 2) {
      // left edge of cube on cursor is cubeSize/2, right edge is -cubeSize/2
      float x0 = points[0].x;
      float x2 = points[2].x;
      float y0 = points[0].y;
      float y2 = points[2].y;

      float middleX = fabsf(x0 + (x2 - x0) * 0.5f);
      // top edge of cube on cursor is 0, bottom edge is cubeSize
      float middleY = fabsf(y0 + (y2 - y0) * 0.5f + cubeSizeHalf); // the +cubeSizeHalf shifts the collision point downwards

      if (middleX < cubeSizeLimit && middleY < cubeSizeLimit && !isInvincible) {
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
            float depthDiff = CUBE_REMOVAL_DEPTH - points[0].z;
            for (int p = 0; p < 20; p++) {
              points[p].z += depthDiff;
            }
          }
          scoreVal += zSpeed;
          if (scoreVal > highScoreVal) {
            newHighScore = true;
            if (!usedDebugMode) {
              highScoreVal = (int)scoreVal;
              writeSaveData();
            }
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
    float absX = fabsf(c.x);
    float absY = fabsf(c.y);
    for (int j = 0; j < 5; j++) {
      if (absX < 2.0f || absY < 2.0f) {
        c.x = randF(-cubeBounds, cubeBounds);
        c.y = randF(-cubeBounds, cubeBounds);
        absX = fabsf(c.x);
        absY = fabsf(c.y);
      } else {
        break;
      }
    }
  }

  static const int pointMapping[12] = {4, 0, 3, 7, 5, 1, 2, 6, 3, 2, 6, 7};
  
  float half = s * 0.5f;
  float cx = c.x;
  float cy = c.y;
  float cz = c.z;
  
  float xNeg = -half + cx;
  float xPos = half + cx;
  float yNeg = -half + cy;
  float yPos = half + cy;
  float zNeg = -half + cz;
  float zPos = s + cz;

  Cube cube;

  // Up
  cube.points[0] = (Point){ xNeg, yNeg, zPos };
  cube.points[1] = (Point){ xPos, yNeg, zPos };
  cube.points[2] = (Point){ xPos, yNeg, zNeg };
  cube.points[3] = (Point){ xNeg, yNeg, zNeg };

  // Down
  cube.points[4] = (Point){ xNeg, yPos, zPos };
  cube.points[5] = (Point){ xPos, yPos, zPos };
  cube.points[6] = (Point){ xPos, yPos, zNeg };
  cube.points[7] = (Point){ xNeg, yPos, zNeg };

  // Left, Right, Front
  for (int p = 0; p < 12; p++) {
    cube.points[8 + p] = cube.points[pointMapping[p]];
  }

  return cube;
}