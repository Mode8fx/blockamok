#include "./draw.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include <stdbool.h>

#include "./game.h"
#include "./math_custom.h"

float MAX_DEPTH;
float HALF_FOV_ANGLE_RADIANS;
float widthMult, heightMult, cubeCollisionCompareX, cubeCollisionCompareY;

SDL_Rect scoreRect;
SDL_Rect gameOverRect;
SDL_Rect titleScreenRect;
SDL_Rect pausedRect;
double scoreVal;

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

TTF_Font *Sans = NULL;
SDL_Color TEXT_COLOR = {0, 0, 0};
SDL_Surface *message_titlescreen_surface;
SDL_Texture *message_titlescreen_texture;
SDL_Surface *message_gameover_surface;
SDL_Texture *message_gameover_texture;
SDL_Surface *message_paused_surface;
SDL_Texture *message_paused_texture;

SDL_Vertex triangle[3];

int faceOrder[5];
SDL_FPoint triable1Points[3][2];
SDL_FPoint triable2Points[3][2];
SDL_Vertex triangle1[3];
SDL_Vertex triangle2[3];

char score[10];

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
    int orgCubeI = f * 4;    // The way our cube is defined, a face has four cornes
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

void initStaticMessages(SDL_Renderer *renderer) {
  Sans = TTF_OpenFont("Mono.ttf", 42 * HEIGHT / 1000);

  message_titlescreen_surface = TTF_RenderText_Solid(Sans, "Blockamok", TEXT_COLOR);
  message_titlescreen_texture = SDL_CreateTextureFromSurface(renderer, message_titlescreen_surface);
  titleScreenRect.x = WIDTH / 2 - (600 * HEIGHT / 1000) / 2;
  titleScreenRect.y = HEIGHT / 2 - (150 * HEIGHT / 1000) / 2 - 10;
  titleScreenRect.w = 600 * HEIGHT / 1000;
  titleScreenRect.h = 150 * HEIGHT / 1000;

  scoreRect.x = 0;
  scoreRect.y = -HEIGHT / 100;
  scoreRect.w = 72 * HEIGHT / 1000;
  scoreRect.h = 50 * HEIGHT / 1000;

  message_gameover_surface = TTF_RenderText_Solid(Sans, "GAME OVER", TEXT_COLOR);
  message_gameover_texture = SDL_CreateTextureFromSurface(renderer, message_gameover_surface);
  gameOverRect.x = WIDTH / 2 - (600 * HEIGHT / 1000) / 2;
  gameOverRect.y = HEIGHT / 2 - (150 * HEIGHT / 1000) / 2 - 10;
  gameOverRect.w = 600 * HEIGHT / 1000;
  gameOverRect.h = 150 * HEIGHT / 1000;

  message_paused_surface = TTF_RenderText_Solid(Sans, "PAUSED", TEXT_COLOR);
  message_paused_texture = SDL_CreateTextureFromSurface(renderer, message_paused_surface);
  pausedRect.x = WIDTH / 2 - (600 * HEIGHT / 1000) / 2;
  pausedRect.y = HEIGHT / 2 - (150 * HEIGHT / 1000) / 2 - 10;
  pausedRect.w = 600 * HEIGHT / 1000;
  pausedRect.h = 150 * HEIGHT / 1000;
}

inline void drawTitleScreenText(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, message_titlescreen_texture, NULL, &titleScreenRect);
}

inline void drawScoreText(SDL_Renderer *renderer) {
  sprintf(score, "%d", (int)scoreVal);
  SDL_Surface *message_score_surface = TTF_RenderText_Solid(Sans, score, TEXT_COLOR);

  // Adjust the position of the scoreRect to center the text
  scoreRect.x = (WIDTH - message_score_surface->w) / 2;
  scoreRect.w = message_score_surface->w;
  scoreRect.h = message_score_surface->h;

  SDL_Texture *message_score_texture = SDL_CreateTextureFromSurface(renderer, message_score_surface);
  SDL_RenderCopy(renderer, message_score_texture, NULL, &scoreRect);
  SDL_FreeSurface(message_score_surface);
  SDL_DestroyTexture(message_score_texture);
}

inline void drawGameOverText(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, message_gameover_texture, NULL, &gameOverRect);
}

inline void drawPausedText(SDL_Renderer *renderer) {
  SDL_RenderCopy(renderer, message_paused_texture, NULL, &pausedRect);
}