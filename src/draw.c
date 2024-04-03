#include "./draw.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdbool.h>

#include "./game.h"
#include "./math_custom.h"

float LARGER_ASPECT_RATIO;
float SMALLER_ASPECT_RATIO;
int MINSCREEN;

float MAX_DEPTH;
float HALF_FOV_ANGLE_RADIANS;

float sizeMult;
SDL_Rect scoreRect;
SDL_Rect gameOverRect;
SDL_Rect titleScreenRect;
SDL_Rect pausedRect;
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

SDL_Color darkBackgroundTriangle = {.r = 0, .b = 0, .g = 0, .a = 250 / 3};
SDL_Color emptyBackgroundTriangle = {.r = 255, .b = 255, .g = 255, .a = 0};

TTF_Font *Sans = NULL;
SDL_Color TEXT_COLOR = {0, 0, 0};

SDL_Vertex triangle[3];
SDL_FPoint triangle1Points[3][2];
SDL_FPoint triangle2Points[3][2];

float x, y, frontStartX, frontEndX, frontStartY, frontEndY;
bool outWithX, outWithY;

int i;

float toDiff;

int f, orgCubeI, transCubeI, p;
Point point;
SDL_Point sPoint;
int faceOrder[5];
int lastI, firstI;
int cubeI;
bool sideOutsideFront;
SDL_FPoint triable1Points[3][2];
SDL_FPoint triable2Points[3][2];
SDL_Color color, c;
float min, max;
float z, fadeAmount;
SDL_Vertex triangle1[3];
SDL_Vertex triangle2[3];
SDL_Point linePoints[5];

char score[10];

SDL_Surface *surfaceMessage;
SDL_Texture *Message;
SDL_Rect Message_rect;

void setScalingVals() {
  if (WIDTH >= HEIGHT) {
    LARGER_ASPECT_RATIO = ((float)WIDTH / HEIGHT);
    SMALLER_ASPECT_RATIO = ((float)HEIGHT / WIDTH);
    MINSCREEN = HEIGHT;
  } else {
    LARGER_ASPECT_RATIO = ((float)HEIGHT / WIDTH);
    SMALLER_ASPECT_RATIO = ((float)WIDTH / HEIGHT);
    MINSCREEN = WIDTH;
  }
  sizeMult = (float)HEIGHT / 1000;
  scoreRect.x = 0;
  scoreRect.y = -HEIGHT / 100;
  scoreRect.w = 72 * HEIGHT / 1000;
  scoreRect.h = 50 * HEIGHT / 1000;
  gameOverRect.x = WIDTH / 2 - (600 * HEIGHT / 1000) / 2;
  gameOverRect.y = HEIGHT / 2 - (150 * HEIGHT / 1000) / 2 - 10;
  gameOverRect.w = 600 * HEIGHT / 1000;
  gameOverRect.h = 150 * HEIGHT / 1000;
  titleScreenRect.x = WIDTH / 2 - (600 * HEIGHT / 1000) / 2;
  titleScreenRect.y = HEIGHT / 2 - (150 * HEIGHT / 1000) / 2 - 10;
  titleScreenRect.w = 600 * HEIGHT / 1000;
  titleScreenRect.h = 150 * HEIGHT / 1000;
  pausedRect.x = WIDTH / 2 - (600 * HEIGHT / 1000) / 2;
  pausedRect.y = HEIGHT / 2 - (150 * HEIGHT / 1000) / 2 - 10;
  pausedRect.w = 600 * HEIGHT / 1000;
  pausedRect.h = 150 * HEIGHT / 1000;

  MAX_DEPTH = 150 * LARGER_ASPECT_RATIO;
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
  triangle1Points[0]->x = -WIDTH;
  triangle1Points[0]->y = HEIGHT / 2;
  triangle1Points[1]->x = WIDTH / 2;
  triangle1Points[1]->y = -HEIGHT;
  triangle1Points[2]->x = WIDTH * 2;
  triangle1Points[2]->y = HEIGHT / 2;
  triangle2Points[0]->x = -WIDTH;
  triangle2Points[0]->y = HEIGHT / 2;
  triangle2Points[1]->x = WIDTH / 2;
  triangle2Points[1]->y = HEIGHT * 2;
  triangle2Points[2]->x = WIDTH * 2;
  triangle2Points[2]->y = HEIGHT / 2;

  SDL_RenderClear(renderer);
  drawBackgroundTriangle(renderer, triangle1Points);
  drawBackgroundTriangle(renderer, triangle2Points);
}

static float screenX(float x) {
  return x * widthMult * WIDTH + WIDTH / 2;
}

static float screenY(float y) {
  return y * heightMult * HEIGHT + HEIGHT / 2;
}

static bool isPointOutsideFront(int f, int frontI) {
  x = transformedCube[f].x;
  y = transformedCube[f].y;
  frontStartX = transformedCube[frontI].x;
  frontEndX = transformedCube[frontI + 2].x;
  frontStartY = transformedCube[frontI].y;
  frontEndY = transformedCube[frontI + 2].y;
  outWithX = x < frontStartX || x > frontEndX;
  outWithY = y < frontStartY || y > frontEndY;
  return outWithX || outWithY;
}

void drawCubes(SDL_Renderer *renderer, Cube cubes[], int cubesLength) {
  for (i = 0; i < cubesLength; i++) {
    drawCube(renderer, cubes[i]);
  }
}

static float fadeTowards(float current, float target, float amount) {
  if (current == target) {
    return current;
  }
  toDiff = fabs((current - target) * amount);
  if (current > target) {
    return current - toDiff;
  } else {
    return current + toDiff;
  }
}

void drawCube(SDL_Renderer *renderer, Cube cube) {
  for (f = 0; f < 5; f++) {
    orgCubeI = f * 4;    // The way our cube is defined, a face has four cornes
    transCubeI = f * 5;  // The way our transformed cube is defined, a face has 5 corners (last one connects back to the firt one)
    for (p = 0; p < 4; p++) {
      point = cube[orgCubeI + p];
      // Changing sPoint.x and sPoint.y can change the "angle" at which you fall, if it looks like you're shifting too much in one direction
      sPoint.x = screenX(transform3Dto2D(point.x, point.z));
      sPoint.y = screenY(transform3Dto2D(point.y, point.z));
      transformedCube[transCubeI + p] = sPoint;
    }
    transformedCube[transCubeI + 4] = transformedCube[transCubeI + 0];
  }

  // If a have has at least two points outside of front, it gets to be drawn last

  lastI = 4;
  firstI = 0;

  faceOrder[lastI--] = FRONT;  // Front always gets to be last

  for (f = 0; f < 4; f++) {
    cubeI = f * 5;
    sideOutsideFront = isPointOutsideFront(cubeI, FRONT * 5) && isPointOutsideFront(cubeI + 1, FRONT * 5);
    // If we are outside, we should draw this as last as possible
    if (sideOutsideFront) {
      faceOrder[lastI--] = f;
    } else {
      faceOrder[firstI++] = f;
    }
  }

  // No need to draw the first 2 faces. They are hidden behind the front
  for (f = 2; f < 5; f++) {
    cubeI = faceOrder[f] * 5;

    triable1Points[0]->x = transformedCube[cubeI + 0].x;
    triable1Points[0]->y = transformedCube[cubeI + 0].y;
    triable1Points[1]->x = transformedCube[cubeI + 1].x;
    triable1Points[1]->y = transformedCube[cubeI + 1].y;
    triable1Points[2]->x = transformedCube[cubeI + 2].x;
    triable1Points[2]->y = transformedCube[cubeI + 2].y;

    triable2Points[0]->x = transformedCube[cubeI + 2].x;
    triable2Points[0]->y = transformedCube[cubeI + 2].y;
    triable2Points[1]->x = transformedCube[cubeI + 3].x;
    triable2Points[1]->y = transformedCube[cubeI + 3].y;
    triable2Points[2]->x = transformedCube[cubeI + 4].x;
    triable2Points[2]->y = transformedCube[cubeI + 4].y;

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

    min = 150;
    max = 190;

    z = (cube[(cubeI / 5) * 4].z) + fabs(cube[(cubeI / 5) * 4].x) * 7 + fabs(cube[(cubeI / 5) * 4].y) * 7;
    fadeAmount = z < min ? 0 : fmin((z - min) / (max - min), 1);

    color.r = color.r;
    color.g = color.g;
    color.b = color.b;
    color.a = fadeTowards(255, 0, fadeAmount);

    triangle1[0].color = color;
    triangle1[1].color = color;
    triangle1[2].color = color;
    triangle2[0].color = color;
    triangle2[1].color = color;
    triangle2[2].color = color;

    triangle1[0].position.x = triable1Points[0]->x;
    triangle1[0].position.y = triable1Points[0]->y;
    triangle1[1].position.x = triable1Points[1]->x;
    triangle1[1].position.y = triable1Points[1]->y;
    triangle1[2].position.x = triable1Points[2]->x;
    triangle1[2].position.y = triable1Points[2]->y;

    triangle2[0].position.x = triable2Points[0]->x;
    triangle2[0].position.y = triable2Points[0]->y;
    triangle2[1].position.x = triable2Points[1]->x;
    triangle2[1].position.y = triable2Points[1]->y;
    triangle2[2].position.x = triable2Points[2]->x;
    triangle2[2].position.y = triable2Points[2]->y;

    linePoints[0] = transformedCube[cubeI + 0];
    linePoints[1] = transformedCube[cubeI + 1];
    linePoints[2] = transformedCube[cubeI + 2];
    linePoints[3] = transformedCube[cubeI + 3];
    linePoints[4] = transformedCube[cubeI + 4];

    SDL_RenderGeometry(renderer, NULL, triangle1, 3, NULL, 0);
    SDL_RenderGeometry(renderer, NULL, triangle2, 3, NULL, 0);
    fadeAmount = fmin(fadeAmount * 1.5, 1);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, fadeTowards(255, 0, fadeAmount));
    SDL_RenderDrawLines(renderer, linePoints, 5);
  }
}

void drawTitleScreenText(SDL_Renderer* renderer) {
  if (Sans == NULL) {
    Sans = TTF_OpenFont("Mono.ttf", 42 * HEIGHT / 1000);
  }
  if (surfaceMessage != NULL) {
    SDL_FreeSurface(surfaceMessage);
    surfaceMessage = NULL;
  }
  if (Message != NULL) {
    SDL_DestroyTexture(Message);
    Message = NULL;
  }
  surfaceMessage = TTF_RenderText_Solid(Sans, "Blockamok", TEXT_COLOR);
  Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
  SDL_RenderCopy(renderer, Message, NULL, &titleScreenRect);
}

void drawSpeedText(SDL_Renderer *renderer) {
  if (surfaceMessage != NULL) {
    SDL_FreeSurface(surfaceMessage);
    surfaceMessage = NULL;
  }
  if (Message != NULL) {
    SDL_DestroyTexture(Message);
    Message = NULL;
  }
  sprintf(score, "%d", (int)playerSpeed);
  surfaceMessage = TTF_RenderText_Solid(Sans, score, TEXT_COLOR);
  Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
  SDL_RenderCopy(renderer, Message, NULL, &scoreRect);
}

void drawGameOverText(SDL_Renderer *renderer) {
  if (surfaceMessage != NULL) {
    SDL_FreeSurface(surfaceMessage);
    surfaceMessage = NULL;
  }
  if (Message != NULL) {
    SDL_DestroyTexture(Message);
    Message = NULL;
  }
  surfaceMessage = TTF_RenderText_Solid(Sans, "GAME OVER", TEXT_COLOR);
  Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
  SDL_RenderCopy(renderer, Message, NULL, &gameOverRect);
}

void drawPausedText(SDL_Renderer *renderer) {
  if (surfaceMessage != NULL) {
    SDL_FreeSurface(surfaceMessage);
    surfaceMessage = NULL;
  }
  if (Message != NULL) {
    SDL_DestroyTexture(Message);
    Message = NULL;
  }
  surfaceMessage = TTF_RenderText_Solid(Sans, "PAUSED", TEXT_COLOR);
  Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
  SDL_RenderCopy(renderer, Message, NULL, &pausedRect);
}