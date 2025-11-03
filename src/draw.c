#include <math.h>
#include <stdbool.h>

#include "./draw.h"
#include "./game.h"
#include "./text.h"

SDL_DisplayMode DM;

SDL_Rect gameViewport;
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

float HEIGHT_DOUBLE;
float HEIGHT_HALF;
float HEIGHT_NEG;
float WIDTH_DOUBLE;
float WIDTH_HALF;
float WIDTH_NEG;
int gameOffsetX;

#define MIN_FADE 150
#define FADE_DIFF 40 // 190 - MIN_FADE
#define FOV_ANGLE 45

void setScalingVals() {
  gameOffsetX = (WINDOW_WIDTH - GAME_WIDTH) / 2;
  gameViewport = (SDL_Rect){ .x = gameOffsetX, .y = 0, .w = GAME_WIDTH, .h = GAME_HEIGHT };
  leftBar = (SDL_Rect){ .x = 0, .y = 0, .w = gameOffsetX, .h = GAME_HEIGHT };
  rightBar = (SDL_Rect){ .x = gameOffsetX + GAME_WIDTH, .y = 0, .w = gameOffsetX + 10, .h = GAME_HEIGHT };
  leftBorder = (SDL_Rect){ .x = 0, .y = 0, .w = GAME_HEIGHT / 100, .h = GAME_HEIGHT };
  leftBorder.x = gameOffsetX - leftBorder.w;
  rightBorder = (SDL_Rect){ .x = gameOffsetX + GAME_WIDTH, .y = 0, .w = GAME_HEIGHT / 100, .h = GAME_HEIGHT };

  HEIGHT_DOUBLE = GAME_HEIGHT * 2.0f;
  HEIGHT_HALF = GAME_HEIGHT * 0.5f;
  HEIGHT_NEG = -(float)GAME_HEIGHT;
  WIDTH_DOUBLE = GAME_WIDTH * 2.0f;
  WIDTH_HALF = GAME_WIDTH * 0.5f;
  WIDTH_NEG = -(float)GAME_WIDTH;

  HALF_FOV_ANGLE_RADIANS = ((FOV_ANGLE / 180.0f) * (float)M_PI) / 2;
  HALF_FOV_ANGLE_RADIANS_TAN = tanf(HALF_FOV_ANGLE_RADIANS);

  triangle[0].color = bgVertexColor1;
  triangle[1].color = bgVertexColor2;
  triangle[2].color = bgVertexColor1;

  drawOverlayOnThisFrame = true;
}

static inline void drawBackgroundTriangle(SDL_Renderer *renderer, SDL_FPoint trianglePoints[]) {
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
  SDL_FPoint triangle1Points[] = {
		{WIDTH_NEG, HEIGHT_HALF},
		{WIDTH_HALF, HEIGHT_NEG},
		{WIDTH_DOUBLE, HEIGHT_HALF}
  };
  SDL_FPoint triangle2Points[] = {
    {WIDTH_NEG, HEIGHT_HALF},
    {WIDTH_HALF, HEIGHT_DOUBLE},
    {WIDTH_DOUBLE, HEIGHT_HALF}
  };
  SDL_RenderClear(renderer);
  drawBackgroundTriangle(renderer, triangle1Points);
  drawBackgroundTriangle(renderer, triangle2Points);
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
  SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
  SDL_RenderClear(renderer);
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

static inline float transform3Dto2D(float xy, float z) {
  return xy / ((z)*HALF_FOV_ANGLE_RADIANS_TAN);
}

static void drawCube(SDL_Renderer *renderer, Cube cube) {
  for (int p = 0; p < 4; p++) {
    Point point = cube.points[p];
    // Transform the 3D point to 2D
    float transformedX = transform3Dto2D(point.x, point.z);
    float transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[4 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[5 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[8 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[10 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[12 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[15 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[16 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[20 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };
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

  if (isPointOutsideFront(0, FRONT_I) && isPointOutsideFront(1, FRONT_I)) {
    faceOrder[lastI--] = 0;
  } else {
    faceOrder[firstI++] = 0;
  }
  if (isPointOutsideFront(5, FRONT_I) && isPointOutsideFront(6, FRONT_I)) {
    faceOrder[lastI--] = 1;
  } else {
    faceOrder[firstI++] = 1;
  }
  if (isPointOutsideFront(10, FRONT_I) && isPointOutsideFront(11, FRONT_I)) {
    faceOrder[lastI--] = 2;
  } else {
    faceOrder[firstI++] = 2;
  }
  if (isPointOutsideFront(15, FRONT_I) && isPointOutsideFront(16, FRONT_I)) {
    faceOrder[lastI--] = 3;
  } else {
    faceOrder[firstI++] = 3;
  }

  // No need to draw the first 2 faces. They are hidden behind the front
  for (int f = 2; f < 5; f++) {
    Sint8 cubeI = faceOrder[f] * 5;

    SDL_Color color = (f == FRONT) ? cubeColorFront : cubeColorSide;
    // Cache cube points and transformed points
    SDL_Point *transformed = &transformedCube[cubeI];

    // Precompute z and fadeAmount
    Sint8 faceIndexMult = faceOrder[f] << 2;
    Point cubePoint = cube.points[faceIndexMult];
    float z = cubePoint.z + fabsf(cubePoint.x) * 7 + fabsf(cubePoint.y) * 7;
    float fadeAmount = (z < MIN_FADE) ? 0 : (z - MIN_FADE) / FADE_DIFF;
    fadeAmount = fminf(fadeAmount, 1.0f);

    color.a = (Uint8)(255 - fadeAmount * 255);

    SDL_Vertex vertices[6];
    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;
    vertices[4].color = color;
    vertices[5].color = color;

    vertices[0].position.x = (float)transformed[0].x;
    vertices[0].position.y = (float)transformed[0].y;
    vertices[1].position.x = (float)transformed[1].x;
    vertices[1].position.y = (float)transformed[1].y;
    vertices[2].position.x = (float)transformed[2].x;
    vertices[2].position.y = (float)transformed[2].y;

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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, (Uint8)(255 - fadeAmount * 255));
    SDL_RenderDrawLines(renderer, transformed, 5);
  }
}

static void drawCubeSimple(SDL_Renderer *renderer, Cube cube) {
  for (int p = 0; p < 4; p++) {
    Point point = cube.points[p];
    // Transform the 3D point to 2D
    float transformedX = transform3Dto2D(point.x, point.z);
    float transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[4 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[5 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[8 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[10 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[12 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[15 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };

    point = cube.points[16 + p];
    // Transform the 3D point to 2D
    transformedX = transform3Dto2D(point.x, point.z);
    transformedY = transform3Dto2D(point.y, point.z);
    // Convert to screen coordinates
    transformedCube[20 + p] = (SDL_Point){
        (int)screenX(transformedX),
        (int)screenY(transformedY)
    };
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

  if (isPointOutsideFront(0, FRONT_I) && isPointOutsideFront(1, FRONT_I)) {
    faceOrder[lastI--] = 0;
  } else {
    faceOrder[firstI++] = 0;
  }
  if (isPointOutsideFront(5, FRONT_I) && isPointOutsideFront(6, FRONT_I)) {
    faceOrder[lastI--] = 1;
  } else {
    faceOrder[firstI++] = 1;
  }
  if (isPointOutsideFront(10, FRONT_I) && isPointOutsideFront(11, FRONT_I)) {
    faceOrder[lastI--] = 2;
  } else {
    faceOrder[firstI++] = 2;
  }
  if (isPointOutsideFront(15, FRONT_I) && isPointOutsideFront(16, FRONT_I)) {
    faceOrder[lastI--] = 3;
  } else {
    faceOrder[firstI++] = 3;
  }

  // No need to draw the first 2 faces. They are hidden behind the front
  for (int f = 2; f < 5; f++) {
    Sint8 cubeI = faceOrder[f] * 5;

    SDL_Color color = (f == FRONT) ? cubeColorFront : cubeColorSide;
    // Cache cube points and transformed points
    SDL_Point *transformed = &transformedCube[cubeI];

    SDL_Vertex vertices[6];
    vertices[0].color = color;
    vertices[1].color = color;
    vertices[2].color = color;
    vertices[3].color = color;
    vertices[4].color = color;
    vertices[5].color = color;

    vertices[0].position.x = (float)transformed[0].x;
    vertices[0].position.y = (float)transformed[0].y;
    vertices[1].position.x = (float)transformed[1].x;
    vertices[1].position.y = (float)transformed[1].y;
    vertices[2].position.x = (float)transformed[2].x;
    vertices[2].position.y = (float)transformed[2].y;

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