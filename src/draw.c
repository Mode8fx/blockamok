#include <math.h>
#include <stdbool.h>

#include "./draw.h"
#include "./game.h"
#include "./text.h"

SDL_DisplayMode DM;

SDL_Rect leftBar;
SDL_Rect rightBar;
SDL_Rect leftBorder;
SDL_Rect rightBorder;

float HALF_FOV_ANGLE_RADIANS;
float HALF_FOV_ANGLE_RADIANS_TAN;

const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int FRONT = 4;
const int FRONT_I = 20; // FRONT * 5

SDL_Texture *backgroundTexture;
SDL_Color backgroundColor = { .r = 15, .g = 255, .b = 155 };
SDL_Color cubeColorFront = { .r = 200, .g = 250, .b = 120, .a = 255 };
SDL_Color cubeColorSide = { .r = 100, .g = 100, .b = 200, .a = 255 };
SDL_Color overlayColor = { .r = 15, .g = 255, .b = 155 };

//int TRANSFORMED_FRONT_I = FRONT * 5;

SDL_Point transformedCube[CUBE_FACE_N * 5];

const SDL_Color bgVertexColor1 = {.r = 0, .g = 0, .b = 0, .a = 250 / 3};
const SDL_Color bgVertexColor2 = {.r = 255, .g = 255, .b = 255, .a = 0};

SDL_Vertex triangle[3];

int faceOrder[5];

float HEIGHT_HALF;
float WIDTH_HALF;
float WINDOW_HEIGHT_DOUBLE;
float WINDOW_HEIGHT_HALF;
float WINDOW_HEIGHT_NEG;
float WINDOW_WIDTH_DOUBLE;
float WINDOW_WIDTH_HALF;
float WINDOW_WIDTH_NEG;
int gameOffsetX;

#define MIN_FADE 150
#define FADE_DIFF 40 // 190 - MIN_FADE
#define FOV_ANGLE 45

void setScalingVals() {
  gameOffsetX = (WINDOW_WIDTH - GAME_WIDTH) / 2;
  
  int borderWidth = GAME_HEIGHT / 100;
  int rightBarX = gameOffsetX + GAME_WIDTH;
  
  leftBar = (SDL_Rect){ .x = 0, .y = 0, .w = gameOffsetX, .h = GAME_HEIGHT };
  rightBar = (SDL_Rect){ .x = rightBarX, .y = 0, .w = gameOffsetX + 10, .h = GAME_HEIGHT };
  leftBorder = (SDL_Rect){ .x = gameOffsetX - borderWidth, .y = 0, .w = borderWidth, .h = GAME_HEIGHT };
  rightBorder = (SDL_Rect){ .x = rightBarX, .y = 0, .w = borderWidth, .h = GAME_HEIGHT };

  float gameHeightF = (float)GAME_HEIGHT;
  float gameWidthF = (float)GAME_WIDTH;
  
  HEIGHT_HALF = gameHeightF * 0.5f;
  WIDTH_HALF = gameWidthF * 0.5f;

  float windowHeightF = (float)WINDOW_HEIGHT;
  float windowWidthF = (float)WINDOW_WIDTH;

  WINDOW_HEIGHT_DOUBLE = windowHeightF * 2.0f;
  WINDOW_HEIGHT_HALF = windowHeightF * 0.5f;
  WINDOW_HEIGHT_NEG = -windowHeightF;
  WINDOW_WIDTH_DOUBLE = windowWidthF * 2.0f;
  WINDOW_WIDTH_HALF = windowWidthF * 0.5f;
  WINDOW_WIDTH_NEG = -windowWidthF;

  HALF_FOV_ANGLE_RADIANS = ((FOV_ANGLE / 180.0f) * (float)M_PI) * 0.5f;
  HALF_FOV_ANGLE_RADIANS_TAN = tanf(HALF_FOV_ANGLE_RADIANS);

  SDL_Color color1 = bgVertexColor1;
  SDL_Color color2 = bgVertexColor2;
  triangle[0].color = color1;
  triangle[1].color = color2;
  triangle[2].color = color1;
}

static inline void drawBackgroundTriangle(SDL_Renderer *renderer, SDL_FPoint *trianglePoints) {
  triangle[0].position = trianglePoints[0];
  triangle[1].position = trianglePoints[1];
  triangle[2].position = trianglePoints[2];
  //triangle[0].position.x += gameOffsetX;
  //triangle[1].position.x += gameOffsetX;
  //triangle[2].position.x += gameOffsetX;
  SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0);
}

inline static void drawBackground(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
  SDL_RenderClear(renderer);
  
  SDL_FPoint trianglePoints[3] = {
    {WINDOW_WIDTH_NEG, WINDOW_HEIGHT_HALF},
    {WINDOW_WIDTH_HALF, WINDOW_HEIGHT_NEG},
    {WINDOW_WIDTH_DOUBLE, WINDOW_HEIGHT_HALF}
  };
  drawBackgroundTriangle(renderer, trianglePoints);
  
  trianglePoints[1].y = WINDOW_HEIGHT_DOUBLE;
  drawBackgroundTriangle(renderer, trianglePoints);
}

inline void draw(SDL_Renderer *renderer) {
#if defined(LOW_SPEC_BG)
  SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
  SDL_RenderClear(renderer);
#elif defined(FORCE_DRAW_BG)
  drawBackground(renderer);
#else
  SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
#endif
}

void saveBackgroundAsTexture(SDL_Renderer *renderer) {
#if !(defined(LOW_SPEC_BG) || defined(FORCE_DRAW_BG))
  SDL_RenderSetViewport(renderer, NULL);
  drawBackground(renderer);
  
  SDL_Surface *screenSurface = SDL_CreateRGBSurfaceWithFormat(0, GAME_WIDTH, GAME_HEIGHT, 24, SDL_PIXELFORMAT_RGB888);
  SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB888, screenSurface->pixels, screenSurface->pitch);
  
  if (backgroundTexture != NULL) {
    SDL_DestroyTexture(backgroundTexture);
  }
  backgroundTexture = SDL_CreateTextureFromSurface(renderer, screenSurface);
  SDL_FreeSurface(screenSurface);
#endif
}

#define screenX(x) (x * GAME_WIDTH + WIDTH_HALF + gameOffsetX)
#define screenY(y) (y * GAME_HEIGHT + HEIGHT_HALF)

static inline bool isPointOutsideFront(int f, int frontI) {
  SDL_Point *point = &transformedCube[f];
  SDL_Point *frontStart = &transformedCube[frontI];
  SDL_Point *frontEnd = &transformedCube[frontI + 2];
  
  return point->x < frontStart->x || point->x > frontEnd->x
      || point->y < frontStart->y || point->y > frontEnd->y;
}

static void drawCube(SDL_Renderer *renderer, Cube cube) {
  // Transform all 20 points from 3D to 2D
  Point *points = cube.points;
  const int destOffsets[5] = {0, 5, 10, 15, 20};
  
  for (int face = 0; face < 5; face++) {
    int srcOffset = face * 4;
    int destOffset = destOffsets[face];
    
    for (int p = 0; p < 4; p++) {
      Point *point = &points[srcOffset + p];
      float invZ = 1.0f / (point->z * HALF_FOV_ANGLE_RADIANS_TAN);
      transformedCube[destOffset + p].x = (int)screenX(point->x * invZ);
      transformedCube[destOffset + p].y = (int)screenY(point->y * invZ);
    }
  }

  // Close the faces by connecting the last points to the first
  transformedCube[4] = transformedCube[0];
  transformedCube[9] = transformedCube[5];
  transformedCube[14] = transformedCube[10];
  transformedCube[19] = transformedCube[15];
  transformedCube[24] = transformedCube[20];

  // If a half has at least two points outside of front, it gets to be drawn last
  int lastI = 4;
  int firstI = 0;
  faceOrder[lastI--] = FRONT;  // Front always gets to be last

  const int faceIndices[4] = {0, 5, 10, 15};
  for (int i = 0; i < 4; i++) {
    int idx = faceIndices[i];
    if (isPointOutsideFront(idx, FRONT_I) && isPointOutsideFront(idx + 1, FRONT_I)) {
      faceOrder[lastI--] = i;
    } else {
      faceOrder[firstI++] = i;
    }
  }

  // No need to draw the first 2 faces. They are hidden behind the front
  for (int f = 2; f < 5; f++) {
    int cubeI = faceOrder[f] * 5;
    SDL_Point *transformed = &transformedCube[cubeI];

    // Select color
    SDL_Color color = (faceOrder[f] == FRONT) ? cubeColorFront : cubeColorSide;

    // Calculate fade amount
    int faceIndexMult = faceOrder[f] << 2;
    Point *cubePoint = &points[faceIndexMult];
    float z = cubePoint->z + fabsf(cubePoint->x) * 7.0f + fabsf(cubePoint->y) * 7.0f;
    float fadeAmount = (z < MIN_FADE) ? 0.0f : fminf((z - MIN_FADE) / FADE_DIFF, 1.0f);
    
    color.a = (Uint8)(255.0f - fadeAmount * 255.0f);

    // Build vertices (two triangles)
    SDL_Vertex vertices[6];
    for (int i = 0; i < 6; i++) {
      vertices[i].color = color;
    }
    
    // First triangle
    vertices[0].position.x = (float)transformed[0].x;
    vertices[0].position.y = (float)transformed[0].y;
    vertices[1].position.x = (float)transformed[1].x;
    vertices[1].position.y = (float)transformed[1].y;
    vertices[2].position.x = (float)transformed[2].x;
    vertices[2].position.y = (float)transformed[2].y;

    // Second triangle
    vertices[3].position.x = (float)transformed[2].x;
    vertices[3].position.y = (float)transformed[2].y;
    vertices[4].position.x = (float)transformed[3].x;
    vertices[4].position.y = (float)transformed[3].y;
    vertices[5].position.x = (float)transformed[4].x;
    vertices[5].position.y = (float)transformed[4].y;

    // Render triangles
    SDL_RenderGeometry(renderer, NULL, vertices, 6, NULL, 0);

    // Render lines with adjusted fadeAmount
    fadeAmount = fminf(fadeAmount * 1.5f, 1.0f);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)(255.0f - fadeAmount * 255.0f));
    SDL_RenderDrawLines(renderer, transformed, 5);
  }
}

static void drawCubeSimple(SDL_Renderer *renderer, Cube cube) {
  // Transform all 20 points from 3D to 2D
  Point *points = cube.points;
  const int destOffsets[5] = {0, 5, 10, 15, 20};

  for (int face = 0; face < 5; face++) {
    int srcOffset = face * 4;
    int destOffset = destOffsets[face];

    for (int p = 0; p < 4; p++) {
      Point* point = &points[srcOffset + p];
      float invZ = 1.0f / (point->z * HALF_FOV_ANGLE_RADIANS_TAN);
      transformedCube[destOffset + p].x = (int)screenX(point->x * invZ);
      transformedCube[destOffset + p].y = (int)screenY(point->y * invZ);
    }
  }

  // Close the faces by connecting the last points to the first
  transformedCube[4] = transformedCube[0];
  transformedCube[9] = transformedCube[5];
  transformedCube[14] = transformedCube[10];
  transformedCube[19] = transformedCube[15];
  transformedCube[24] = transformedCube[20];

  // If a half has at least two points outside of front, it gets to be drawn last
  int lastI = 4;
  int firstI = 0;
  faceOrder[lastI--] = FRONT;  // Front always gets to be last

  const int faceIndices[4] = { 0, 5, 10, 15 };
  for (int i = 0; i < 4; i++) {
    int idx = faceIndices[i];
    if (isPointOutsideFront(idx, FRONT_I) && isPointOutsideFront(idx + 1, FRONT_I)) {
      faceOrder[lastI--] = i;
    }
    else {
      faceOrder[firstI++] = i;
    }
  }

  // No need to draw the first 2 faces. They are hidden behind the front
  for (int f = 2; f < 5; f++) {
    int cubeI = faceOrder[f] * 5;
    SDL_Point *transformed = &transformedCube[cubeI];

    // Select color
    SDL_Color color = (faceOrder[f] == FRONT) ? cubeColorFront : cubeColorSide;

    // Build vertices (two triangles)
    SDL_Vertex vertices[6];
    for (int i = 0; i < 6; i++) {
      vertices[i].color = color;
    }

    // First triangle
    vertices[0].position.x = (float)transformed[0].x;
    vertices[0].position.y = (float)transformed[0].y;
    vertices[1].position.x = (float)transformed[1].x;
    vertices[1].position.y = (float)transformed[1].y;
    vertices[2].position.x = (float)transformed[2].x;
    vertices[2].position.y = (float)transformed[2].y;

    // Second triangle
    vertices[3].position.x = (float)transformed[2].x;
    vertices[3].position.y = (float)transformed[2].y;
    vertices[4].position.x = (float)transformed[3].x;
    vertices[4].position.y = (float)transformed[3].y;
    vertices[5].position.x = (float)transformed[4].x;
    vertices[5].position.y = (float)transformed[4].y;

    // Render triangles
    SDL_RenderGeometry(renderer, NULL, vertices, 6, NULL, 0);

    // Render lines
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLines(renderer, transformed, 5);
  }
}

void drawEssentials(SDL_Renderer *renderer, Cube cubes[], int cubesLength) {
  draw(renderer);

  if (OPTION_SIMPLE_CUBES) {
    for (int i = 0; i < cubesLength; i++) {
      drawCubeSimple(renderer, cubes[i]);
    }
  } else {
    for (int i = 0; i < cubesLength; i++) {
      drawCube(renderer, cubes[i]);
    }
  }
}