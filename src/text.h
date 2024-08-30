#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define TEXT_LINE_SIZE 50

typedef struct {
	char text[TEXT_LINE_SIZE];
	SDL_Rect text_rect;
	SDL_Rect outline_rect;
	SDL_Texture *text_texture;
	SDL_Texture *outline_texture;
} Message;

extern bool credits_paused;
extern Uint64 credits_startTime;

extern void initStaticMessages(SDL_Renderer *renderer);
extern void drawTitleScreenText(SDL_Renderer *renderer, bool drawSecondaryText);
extern void drawInstructionsText(SDL_Renderer* renderer);
extern void drawCreditsText(SDL_Renderer *renderer, Uint64 timer);
extern void drawScoreText(SDL_Renderer *renderer);
extern void drawGameOverText(SDL_Renderer *renderer);
extern void drawPausedText(SDL_Renderer *renderer);
