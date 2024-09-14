#include <math.h>
#include <stdbool.h>

#include "./draw.h"
#include "./game.h"
#include "./math_custom.h"

SDL_DisplayMode DM;

SDL_Rect gameViewport;
SDL_Rect leftBar;
SDL_Rect rightBar;

float HALF_FOV_ANGLE_RADIANS;

const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int FRONT = 4;

SDL_Color backgroundColor = { .r = 15, .g = 255, .b = 155 };

SDL_Color cubeColorFront = { .r = 200, .g = 250, .b = 120 };
SDL_Color cubeColorSide = { .r = 100, .g = 100, .b = 200 };

//int TRANSFORMED_FRONT_I = FRONT * 5;

SDL_Point transformedCube[CUBE_FACE_N * 5];

const SDL_Color darkBackgroundTriangle = {.r = 0, .b = 0, .g = 0, .a = 250 / 3};
const SDL_Color emptyBackgroundTriangle = {.r = 255, .b = 255, .g = 255, .a = 0};

SDL_Vertex triangle[3];

int faceOrder[5];
SDL_Vertex triangle1[3];
SDL_Vertex triangle2[3];

float HEIGHT_DOUBLE;
float HEIGHT_HALF;
float HEIGHT_NEG;
float WIDTH_DOUBLE;
float WIDTH_HALF;
float WIDTH_NEG;

#define MIN_FADE 150
#define MAX_FADE 190

void setScalingVals() {
  int gameOffsetX = (WINDOW_WIDTH - GAME_WIDTH) / 2;
  gameViewport = (SDL_Rect){ .x = gameOffsetX, .y = 0, .w = GAME_WIDTH, .h = GAME_HEIGHT };
  leftBar = (SDL_Rect){ .x = 0, .y = 0, .w = gameOffsetX, .h = WINDOW_HEIGHT };
  rightBar = (SDL_Rect){ .x = gameOffsetX + GAME_WIDTH, .y = 0, .w = gameOffsetX + 10, .h = WINDOW_HEIGHT };

  HEIGHT_DOUBLE = GAME_HEIGHT * 2.0f;
  HEIGHT_HALF = GAME_HEIGHT * 0.5f;
  HEIGHT_NEG = -(float)GAME_HEIGHT;
  WIDTH_DOUBLE = GAME_WIDTH * 2.0f;
  WIDTH_HALF = GAME_WIDTH * 0.5f;
  WIDTH_NEG = -(float)GAME_WIDTH;

  HALF_FOV_ANGLE_RADIANS = ((FOV_ANGLE / 180.0f) * (float)M_PI) / 2;

  triangle[0].color = darkBackgroundTriangle;
  triangle[1].color = emptyBackgroundTriangle;
  triangle[2].color = darkBackgroundTriangle;
}

static inline void drawBackgroundTriangle(SDL_Renderer *renderer, SDL_FPoint trianglePoints[]) {
  triangle[0].position = trianglePoints[0];
  triangle[1].position = trianglePoints[1];
  triangle[2].position = trianglePoints[2];
  SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0);
}

inline void draw(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
  SDL_FPoint triangle1Points[3][2] = {
		{WIDTH_NEG, HEIGHT_HALF},
		{WIDTH_HALF, HEIGHT_NEG},
		{WIDTH_DOUBLE, HEIGHT_HALF}
  };
  SDL_FPoint triangle2Points[3][2] = {
    {WIDTH_NEG, HEIGHT_HALF},
    {WIDTH_HALF, HEIGHT_DOUBLE},
    {WIDTH_DOUBLE, HEIGHT_HALF}
  };

  SDL_RenderClear(renderer);
  drawBackgroundTriangle(renderer, *triangle1Points);
  drawBackgroundTriangle(renderer, *triangle2Points);
}

static inline float screenX(float x) {
  return x * GAME_WIDTH + WIDTH_HALF;
}

static inline float screenY(float y) {
  return y * GAME_HEIGHT + HEIGHT_HALF;
}

static bool isPointOutsideFront(int f, int frontI) {
  //int x = transformedCube[f].x;
  //int y = transformedCube[f].y;
  //int frontStartX = transformedCube[frontI].x;
  //int frontEndX = transformedCube[frontI + 2].x;
  //int frontStartY = transformedCube[frontI].y;
  //int frontEndY = transformedCube[frontI + 2].y;

  //return (x < frontStartX || x > frontEndX || y < frontStartY || y > frontEndY);

  return transformedCube[f].x < transformedCube[frontI].x || transformedCube[f].x > transformedCube[frontI + 2].x
    || transformedCube[f].y < transformedCube[frontI].y || transformedCube[f].y > transformedCube[frontI + 2].y;
}

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength) {
  for (int i = 0; i < cubesLength; i++) {
    drawCube(renderer, cubes[i]);
  }
}

inline void drawEssentials(SDL_Renderer *renderer, Cube cubes[], int cubesLength) {
  draw(renderer);
  drawCubes(renderer, cubes, cubesLength);
}

inline void drawEssentialsWithAlpha(SDL_Renderer *renderer, Cube cubes[], int cubesLength, Uint8 alpha) {
  draw(renderer);
  drawCubes(renderer, cubes, cubesLength);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
  SDL_Rect rect = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
  SDL_RenderFillRect(renderer, &rect);
}

static inline float fadeTowards(float current, float target, float amount) {
  float diff = (target - current) * amount;
  return current + diff;
}

void drawCube(SDL_Renderer *renderer, Cube cube) {
  for (int f = 0; f < 5; f++) {
    Sint8 orgCubeI = f * 4;    // The way our cube is defined, a face has four corners
    Sint8 transCubeI = f * 5;  // The way our transformed cube is defined, a face has 5 corners (last one connects back to the first one)
    for (Sint8 p = 0; p < 4; p++) {
      Point point = cube[orgCubeI + p];
      // Changing sPoint.x and sPoint.y can change the "angle" at which you fall, if it looks like you're shifting too much in one direction
      SDL_Point sPoint = {
        (int)screenX(transform3Dto2D(point.x, point.z)),
        (int)screenY(transform3Dto2D(point.y, point.z))
      };
      transformedCube[transCubeI + p] = sPoint;
    }
    transformedCube[transCubeI + 4] = transformedCube[transCubeI + 0];
  }

  // If a have has at least two points outside of front, it gets to be drawn last

  int lastI = 4;
  int firstI = 0;

  faceOrder[lastI--] = FRONT;  // Front always gets to be last

  for (int f = 0; f < 4; f++) {
    int cubeI = f * 5;
    bool sideOutsideFront = isPointOutsideFront(cubeI, FRONT * 5) && isPointOutsideFront(cubeI + 1, FRONT * 5);
    // If we are outside, we should draw this as last as possible
    if (sideOutsideFront) {
      faceOrder[lastI--] = f;
    } else {
      faceOrder[firstI++] = f;
    }
  }

  // No need to draw the first 2 faces. They are hidden behind the front
  for (int f = 2; f < 5; f++) {
    Sint8 faceIndexMult = faceOrder[f] * 4;
    Sint8 cubeI = faceOrder[f] * 5;

    SDL_Color color;
    if (f == FRONT) {
      color = cubeColorFront;
    } else {
      color = cubeColorSide;
    }

    float z = (cube[faceIndexMult].z) + fabsf(cube[faceIndexMult].x) * 7 + fabsf(cube[faceIndexMult].y) * 7;
    float fadeAmount = z < MIN_FADE ? 0 : fminf((z - MIN_FADE) / (MAX_FADE - MIN_FADE), 1.0f);

    color.a = (Uint8)fadeTowards(255, 0, fadeAmount);

    triangle1[0].color = color;
    triangle1[1].color = color;
    triangle1[2].color = color;
    triangle2[0].color = color;
    triangle2[1].color = color;
    triangle2[2].color = color;

    triangle1[0].position.x = (float)transformedCube[cubeI].x;
    triangle1[0].position.y = (float)transformedCube[cubeI].y;
    triangle1[1].position.x = (float)transformedCube[cubeI + 1].x;
    triangle1[1].position.y = (float)transformedCube[cubeI + 1].y;
    triangle1[2].position.x = (float)transformedCube[cubeI + 2].x;
    triangle1[2].position.y = (float)transformedCube[cubeI + 2].y;

    triangle2[0].position.x = (float)transformedCube[cubeI + 2].x;
    triangle2[0].position.y = (float)transformedCube[cubeI + 2].y;
    triangle2[1].position.x = (float)transformedCube[cubeI + 3].x;
    triangle2[1].position.y = (float)transformedCube[cubeI + 3].y;
    triangle2[2].position.x = (float)transformedCube[cubeI + 4].x;
    triangle2[2].position.y = (float)transformedCube[cubeI + 4].y;

    SDL_Point linePoints[] = {
      transformedCube[cubeI],
      transformedCube[cubeI + 1],
      transformedCube[cubeI + 2],
      transformedCube[cubeI + 3],
      transformedCube[cubeI + 4]
    };

    SDL_RenderGeometry(renderer, NULL, triangle1, 3, NULL, 0);
    SDL_RenderGeometry(renderer, NULL, triangle2, 3, NULL, 0);
    fadeAmount = fminf(fadeAmount * 1.5f, 1.0f);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)fadeTowards(255, 0, fadeAmount));
    SDL_RenderDrawLines(renderer, linePoints, 5);
  }
}