#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

#define MAX_MESSAGE_SIZE 20 // most text lines use per-character Messages, not per-line Messages
#define TEXT_LINE_SIZE 50

typedef struct {
	char text[MAX_MESSAGE_SIZE];
	SDL_Rect text_rect;
	SDL_Rect outline_rect;
	SDL_Texture *text_texture;
	SDL_Texture *outline_texture;
} Message;

typedef struct {
	char name[TEXT_LINE_SIZE];
	//float x; same for all choices
	float y;
	char descLine1[TEXT_LINE_SIZE];
	char descLine2[TEXT_LINE_SIZE];
	char descLine3[TEXT_LINE_SIZE];
} OptionChoice;

typedef struct {
	char name[TEXT_LINE_SIZE];
	//float x; same for all lines
	float y;
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

#define NUM_PRINTABLE_CHARS 95 // From ASCII 32 to 126
#define FIRST_PRINTABLE_CHAR 32
extern Message message_characters_white_38[NUM_PRINTABLE_CHARS];
extern Message message_characters_gray_38[NUM_PRINTABLE_CHARS];

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
#define OPTION_SPAWN_AREA optionPage_Game.optionLines[4].index

#define OPTION_BACKGROUND_COLOR optionPage_Visual.optionLines[0].index
#define OPTION_CUBE_COLOR optionPage_Visual.optionLines[1].index
#define OPTION_OVERLAY_COLOR optionPage_Visual.optionLines[2].index
#define OPTION_SIMPLE_CUBES optionPage_Visual.optionLines[3].index
#define OPTION_SPEEDOMETER optionPage_Visual.optionLines[4].index
#define OPTION_FRAME_RATE optionPage_Visual.optionLines[5].index
#define OPTION_FULLSCREEN optionPage_Visual.optionLines[6].index

#define OPTION_MUSIC optionPage_Audio.optionLines[0].index
#define OPTION_MUSIC_VOLUME optionPage_Audio.optionLines[1].index
#define OPTION_SFX_VOLUME optionPage_Audio.optionLines[2].index

extern bool forceIndexReset;
extern Uint16 frameRate;
extern Uint16 displayRefreshRate;
extern Uint32 ticksPerFrame;

extern void prepareMessage(SDL_Renderer *renderer, TTF_Font *font, int outlineSize, Message *message, float sizeMult, SDL_Color textColor, SDL_Color outlineColor);
extern void renderMessage(SDL_Renderer *renderer, Message *message);
extern void setMessagePosRelativeToScreenY(Message *message, float y);
extern void setMessagePosRelativeToScreen(Message *message, float x, float y);
extern void setMessagePosRelativeToScreen_LeftAlign(Message *message, float x, float y);

extern void initStaticMessages(SDL_Renderer *renderer);
extern void drawTitleScreenText(SDL_Renderer *renderer, bool drawSecondaryText);
extern void drawInstructionsText(SDL_Renderer* renderer);
extern void drawCreditsText(SDL_Renderer *renderer, Uint32 timer);
extern void drawResetHighScoreText(SDL_Renderer *renderer);
extern void drawQuitText(SDL_Renderer *renderer);
extern void drawGameText(SDL_Renderer *renderer);
extern void drawCursor(SDL_Renderer *renderer);
extern void drawGameOverText(SDL_Renderer *renderer);
extern void drawPausedText(SDL_Renderer *renderer);
extern void cleanUpText();

extern void initStaticMessages_Options(SDL_Renderer *renderer);
extern void openPage(SDL_Renderer *renderer, OptionPage *page, bool resetIndex);
extern void optionCallback_CubeFrequency(SDL_Renderer *renderer);
extern void optionCallback_Fullscreen(SDL_Window *window, OptionPage *page);
extern void optionCallback_OverlayColor();
extern void optionCallback_All();
extern void handlePage(SDL_Renderer *renderer, SDL_Window *window, OptionPage *page, bool renderCursor);
extern void cleanUpMenu();
extern void printFPS();
