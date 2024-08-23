#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

typedef struct {
	char text[100];
	SDL_Rect rect;
	SDL_Surface *surface;
	SDL_Texture *texture;
} Message;

extern inline void initStaticMessages(SDL_Renderer *renderer);
extern inline void drawTitleScreenText(SDL_Renderer *renderer);
extern inline void drawCreditsText(SDL_Renderer *renderer);
extern inline void drawScoreText(SDL_Renderer *renderer);
extern inline void drawGameOverText(SDL_Renderer *renderer);
extern inline void drawPausedText(SDL_Renderer *renderer);
