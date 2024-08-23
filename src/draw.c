#include "./draw.h"

#include <math.h>
#include <stdbool.h>

#include "./game.h"
#include "./math_custom.h"

#if !defined(SDL1)
SDL_DisplayMode DM;
#endif

float MAX_DEPTH;
float HALF_FOV_ANGLE_RADIANS;
float widthMult, heightMult, cubeCollisionCompareX, cubeCollisionCompareY;

const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int FRONT = 4;

const int BACKGROUND_R = 15;
const int BACKGROUND_G = 255;
const int BACKGROUND_B = 155;

//int TRANSFORMED_FRONT_I = FRONT * 5;

SDL_Point transformedCube[CUBE_FACE_N * 5];

const SDL_Color darkBackgroundTriangle = {.r = 0, .b = 0, .g = 0, .a = 250 / 3};
const SDL_Color emptyBackgroundTriangle = {.r = 255, .b = 255, .g = 255, .a = 0};

SDL_Vertex triangle[3];

int faceOrder[5];
SDL_Vertex triangle1[3];
SDL_Vertex triangle2[3];

const float HEIGHT_DOUBLE = HEIGHT * 2.0f;
const float HEIGHT_HALF = HEIGHT * 0.5f;
const float HEIGHT_NEG = -HEIGHT;
const float WIDTH_DOUBLE = WIDTH * 2.0f;
const float WIDTH_HALF = WIDTH * 0.5f;
const float WIDTH_NEG = -WIDTH;

void setScalingVals() {
  float largerAspectRatio;
  float smallerAspectRatio;
  if (WIDTH >= HEIGHT) {
    largerAspectRatio = ((float)WIDTH / HEIGHT);
    smallerAspectRatio = ((float)HEIGHT / WIDTH);
  } else {
    largerAspectRatio = ((float)HEIGHT / WIDTH);
    smallerAspectRatio = ((float)WIDTH / HEIGHT);
  }

  MAX_DEPTH = 150 * largerAspectRatio;
  HALF_FOV_ANGLE_RADIANS = ((ADJUSTED_FOV / 180.0) * M_PI) / 2;
}

static void drawBackgroundTriangle(SDL_Renderer *renderer, SDL_FPoint trianglePoints[]) {
  triangle[0].position = trianglePoints[0];
  triangle[1].position = trianglePoints[1];
  triangle[2].position = trianglePoints[2];
  triangle[0].color = darkBackgroundTriangle;
  triangle[1].color = emptyBackgroundTriangle;
  triangle[2].color = darkBackgroundTriangle;
  SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0);
}

void draw(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, BACKGROUND_R, BACKGROUND_G, BACKGROUND_B, 255);
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
  return x * widthMult * WIDTH + WIDTH_HALF;
}

static inline float screenY(float y) {
  return y * heightMult * HEIGHT + HEIGHT_HALF;
}

static bool isPointOutsideFront(int f, int frontI) {
  float x = transformedCube[f].x;
  float y = transformedCube[f].y;
  float frontStartX = transformedCube[frontI].x;
  float frontEndX = transformedCube[frontI + 2].x;
  float frontStartY = transformedCube[frontI].y;
  float frontEndY = transformedCube[frontI + 2].y;
  bool outWithX = x < frontStartX || x > frontEndX;
	if (outWithX) {
		return true;
	}
  bool outWithY = y < frontStartY || y > frontEndY;
  if (outWithY) {
    return true;
  }
  return false;
}

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength) {
  for (int i = 0; i < cubesLength; i++) {
    drawCube(renderer, cubes[i]);
  }
}

static inline float fadeTowards(float current, float target, float amount) {
  if (current == target) {
    return current;
  }
  float toDiff = fabsf((current - target) * amount);
  if (current > target) {
    return current - toDiff;
  } else {
    return current + toDiff;
  }
}

void drawCube(SDL_Renderer *renderer, Cube cube) {
  for (int f = 0; f < 5; f++) {
    int orgCubeI = f * 4;    // The way our cube is defined, a face has four corners
    int transCubeI = f * 5;  // The way our transformed cube is defined, a face has 5 corners (last one connects back to the firt one)
    for (int p = 0; p < 4; p++) {
      Point point = cube[orgCubeI + p];
      // Changing sPoint.x and sPoint.y can change the "angle" at which you fall, if it looks like you're shifting too much in one direction
      SDL_Point sPoint = {
        screenX(transform3Dto2D(point.x, point.z)),
        screenY(transform3Dto2D(point.y, point.z))
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
    int cubeI = faceOrder[f] * 5;

    SDL_Color color, c;
    if (f == FRONT) {
      c.r = 200;
      c.b = 120;
      c.g = 250;
      color = c;
    } else {
      c.r = 100;
      c.b = 200;
      c.g = 100;
      color = c;
    }

    float min = 150;
    float max = 190;

    float z = (cube[(cubeI / 5) * 4].z) + fabsf(cube[(cubeI / 5) * 4].x) * 7 + fabsf(cube[(cubeI / 5) * 4].y) * 7;
    float fadeAmount = z < min ? 0 : fminf((z - min) / (max - min), 1);

    color.a = fadeTowards(255, 0, fadeAmount);

    triangle1[0].color = color;
    triangle1[1].color = color;
    triangle1[2].color = color;
    triangle2[0].color = color;
    triangle2[1].color = color;
    triangle2[2].color = color;

    triangle1[0].position.x = (float)transformedCube[cubeI + 0].x;
    triangle1[0].position.y = (float)transformedCube[cubeI + 0].y;
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
      transformedCube[cubeI + 0],
      transformedCube[cubeI + 1],
      transformedCube[cubeI + 2],
      transformedCube[cubeI + 3],
      transformedCube[cubeI + 4]
    };

    SDL_RenderGeometry(renderer, NULL, triangle1, 3, NULL, 0);
    SDL_RenderGeometry(renderer, NULL, triangle2, 3, NULL, 0);
    fadeAmount = fminf(fadeAmount * 1.5, 1);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, fadeTowards(255, 0, fadeAmount));
    SDL_RenderDrawLines(renderer, linePoints, 5);
  }
}