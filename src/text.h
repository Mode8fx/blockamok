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

typedef struct {
	Message name;
	Message descLine1;
	Message descLine2;
	Message descLine3;
} OptionChoice;

typedef struct {
	Message name;
	Sint8 numChoices;
	Sint8 index;
	OptionChoice *optionChoices; // if there are no options, use a single empty Option
	int nextState; // if - 1, ignore
	bool oneDesc;
} OptionLine;

typedef struct {
	Sint8 pageID;
	Sint8 numLines;
	Sint8 index;
	OptionLine *optionLines;
	int prevState;
} OptionPage;

extern TTF_Font *Sans_126;
extern int outlineSize_126;
extern TTF_Font *Sans_42;
extern int outlineSize_42;
extern TTF_Font *Sans_38;
extern int outlineSize_38;
extern SDL_Color color_black;
extern SDL_Color color_white;
extern SDL_Color color_gray;
extern SDL_Color color_orange;
extern SDL_Color color_red;
extern SDL_Color color_blue;

extern bool credits_paused;
extern Uint32 credits_startTime;

extern OptionPage optionPage_Main;
extern OptionPage optionPage_Game;
extern OptionPage optionPage_Visual;
extern OptionPage optionPage_Audio;
extern OptionPage optionPage_Empty;

#define OPTION_CUBE_FREQUENCY optionPage_Game.optionLines[0].index
#define OPTION_CUBE_SIZE optionPage_Game.optionLines[1].index
#define OPTION_LIVES optionPage_Game.optionLines[2].index
#define OPTION_CONTROL_TYPE optionPage_Game.optionLines[3].index

#define OPTION_BACKGROUND_COLOR optionPage_Visual.optionLines[0].index
#define OPTION_CUBE_COLOR optionPage_Visual.optionLines[1].index
#define OPTION_FULLSCREEN optionPage_Visual.optionLines[2].index

#define OPTION_MUSIC optionPage_Audio.optionLines[0].index
#define OPTION_MUSIC_VOLUME optionPage_Audio.optionLines[1].index
#define OPTION_SFX_VOLUME optionPage_Audio.optionLines[2].index

extern bool forceIndexReset;

extern void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, int outlineSize, Message *message, float sizeMult, SDL_Color textColor, SDL_Color outlineColor);
extern void renderMessage(SDL_Renderer *renderer, Message *message);
extern void setMessagePosRelativeToScreen(Message *message, float x, float y);
extern void setMessagePosRelativeToScreen_LeftAlign(Message *message, float x, float y);

extern void initStaticMessages(SDL_Renderer *renderer);
extern void drawTitleScreenText(SDL_Renderer *renderer, bool drawSecondaryText);
extern void drawInstructionsText(SDL_Renderer* renderer);
extern void drawCreditsText(SDL_Renderer *renderer, Uint32 timer);
extern void drawResetHighScoreText(SDL_Renderer *renderer);
extern void drawQuitText(SDL_Renderer *renderer);
extern void drawScoreAndLivesText(SDL_Renderer *renderer);
extern void drawCursor(SDL_Renderer *renderer);
extern void drawGameOverText(SDL_Renderer *renderer);
extern void drawPausedText(SDL_Renderer *renderer);
extern void refreshHighScoreText(SDL_Renderer *renderer);
extern void cleanUpText();

extern void initStaticMessages_Options(SDL_Renderer *renderer);
extern void openPage(SDL_Renderer *renderer, OptionPage *page, bool resetIndex);
extern void optionCallback_Fullscreen(SDL_Window *window, OptionPage *page);
extern void optionCallback_All();
extern void handlePage(SDL_Renderer *renderer, SDL_Window *window, OptionPage *page, bool renderCursor);
extern void cleanUpMenu();
