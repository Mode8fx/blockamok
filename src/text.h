#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

typedef struct {
	char text[100];
	SDL_Rect text_rect;
	SDL_Rect outline_rect;
	SDL_Texture *text_texture;
	SDL_Texture *outline_texture;
} Message;

extern void initStaticMessages(SDL_Renderer *renderer);
extern void drawTitleScreenText(SDL_Renderer *renderer);
extern void drawInstructionsText(SDL_Renderer* renderer);
extern void drawCreditsText(SDL_Renderer *renderer);
extern void drawScoreText(SDL_Renderer *renderer);
extern void drawGameOverText(SDL_Renderer *renderer);
extern void drawPausedText(SDL_Renderer *renderer);
