﻿#include <stdio.h>

#include "./text.h"
#include "./game.h"
#include "./input.h"
#include "./config.h"
#include "./draw.h"
#include "./audio.h"

#define EMPTY " "
#define CURSOR_X 0.1f
#define CURSOR_Y (0.1f + 0.075f * page->index)
#define LINE_X 0.15f
#define LINE_Y (0.1f + 0.075f * lineIndex)
#define CHOICE_X 0.6f
#define CHOICE_Y LINE_Y
#define DESC_LINE_Y_POS_1 0.78f
#define DESC_LINE_Y_POS_2 0.81f
#define DESC_LINE_Y_POS_3 0.84f
#define DESC_LINE_Y_POS_4 0.87f
#define DESC_LINE_Y_POS_5 0.9f
#define OPTION_FONT Sans_38
#define OPTION_OUTLINE_SIZE outlineSize_38

Message message_menu_cursor;

OptionPage optionPage_Main;
#if !defined(NO_QUIT)
#define OPTION_PAGE_MAIN_NUM_LINES 8
#else
#define OPTION_PAGE_MAIN_NUM_LINES 7
#endif
OptionLine optionPage_Main_Lines[OPTION_PAGE_MAIN_NUM_LINES];

OptionPage optionPage_Game;
#define OPTION_PAGE_GAME_NUM_LINES 4
OptionLine optionPage_Game_Lines[OPTION_PAGE_GAME_NUM_LINES];

OptionPage optionPage_Visual;
#if defined(PC)
#define OPTION_PAGE_VISUAL_NUM_LINES 7
#else
#define OPTION_PAGE_VISUAL_NUM_LINES 6
#endif
OptionLine optionPage_Visual_Lines[OPTION_PAGE_VISUAL_NUM_LINES];

OptionPage optionPage_Audio;
#define OPTION_PAGE_AUDIO_NUM_LINES 3
OptionLine optionPage_Audio_Lines[OPTION_PAGE_AUDIO_NUM_LINES];

OptionPage optionPage_Empty;
OptionLine optionPage_Empty_Lines[1];
#define STAY -1

bool forceIndexReset = false;
Uint16 frameRate = 60;
Uint16 displayRefreshRate;
Uint32 ticksPerFrame;

static void setOptionPageLine(SDL_Renderer *renderer, OptionPage *page, int lineIndex, char text[], Sint8 numChoices, Sint8 choiceIndex, int nextState, bool oneDesc) {
	OptionLine *currLine = &page->optionLines[lineIndex];
	currLine->y = LINE_Y;
	snprintf(currLine->name, TEXT_LINE_SIZE, "%s", text);
	currLine->numChoices = numChoices;
	if (currLine->index == 0 || (forceIndexReset) && !(page->pageID == 4 && lineIndex == 0)) { // don't reset music index
		currLine->index = choiceIndex;
	}
	if (currLine->optionChoices == NULL) {
		currLine->optionChoices = (OptionChoice*)malloc(numChoices * sizeof(OptionChoice));
		for (int i = 0; i < numChoices; i++) {
			currLine->optionChoices[i] = (OptionChoice){ 0 };
		}
	}
	currLine->nextState = nextState;
	currLine->oneDesc = oneDesc;
}

static void setOptionChoice(SDL_Renderer *renderer, OptionPage *page, int lineIndex, int choiceIndex, char name[], char descLine1[], char descLine2[], char descLine3[]) {
	OptionChoice *currChoice = &page->optionLines[lineIndex].optionChoices[choiceIndex];
	currChoice->y = CHOICE_Y;
	snprintf(currChoice->name, TEXT_LINE_SIZE, "%s", name);
	snprintf(currChoice->descLine1, TEXT_LINE_SIZE, "%s", descLine1);
	snprintf(currChoice->descLine2, TEXT_LINE_SIZE, "%s", descLine2);
	snprintf(currChoice->descLine3, TEXT_LINE_SIZE, "%s", descLine3);
}

void initStaticMessages_Options(SDL_Renderer *renderer) {
	bool compactView = GAME_HEIGHT <= 289;

	snprintf(message_menu_cursor.text, MAX_MESSAGE_SIZE, ">");
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &message_menu_cursor, 1, color_white, color_black);

	optionPage_Main.pageID = 0;
	optionPage_Main.numLines = OPTION_PAGE_MAIN_NUM_LINES;
	optionPage_Main.optionLines = optionPage_Main_Lines;
	optionPage_Main.prevState = GAME_STATE_TITLE_SCREEN;
	setOptionPageLine(renderer, &optionPage_Main, 0, "Game Options", 1, 0, GAME_STATE_OPTIONS_GAME, true);
	setOptionChoice(renderer,   &optionPage_Main, 0, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 1, "Visuals", 1, 0, GAME_STATE_OPTIONS_VISUAL, true);
	setOptionChoice(renderer,   &optionPage_Main, 1, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 2, "Audio", 1, 0, GAME_STATE_OPTIONS_AUDIO, true);
	setOptionChoice(renderer,   &optionPage_Main, 2, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 3, "Instructions", 1, 0, GAME_STATE_INSTRUCTIONS, true);
	setOptionChoice(renderer,   &optionPage_Main, 3, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 4, "Credits", 1, 0, GAME_STATE_CREDITS, true);
	setOptionChoice(renderer,   &optionPage_Main, 4, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 5, "Reset High Score", 1, 0, GAME_STATE_RESET_HIGH_SCORE, true);
	setOptionChoice(renderer,   &optionPage_Main, 5, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 6, "Reset Settings", 1, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Main, 6, 0, EMPTY, EMPTY, EMPTY, EMPTY);
#if !defined(NO_QUIT)
	setOptionPageLine(renderer, &optionPage_Main, 7, "Quit", 1, 0, GAME_STATE_QUIT, true);
	setOptionChoice(renderer,   &optionPage_Main, 7, 0, EMPTY, EMPTY, EMPTY, EMPTY);
#endif

	optionPage_Empty.pageID = 1;
	optionPage_Empty.optionLines = optionPage_Empty_Lines;
	optionPage_Empty.prevState = GAME_STATE_OPTIONS_MAIN;
	setOptionPageLine(renderer, &optionPage_Empty, 0, EMPTY, 1, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Empty, 0, 0, EMPTY, EMPTY, EMPTY, EMPTY);

	optionPage_Game.pageID = 2;
	optionPage_Game.numLines = OPTION_PAGE_GAME_NUM_LINES;
	optionPage_Game.optionLines = optionPage_Game_Lines;
	optionPage_Game.prevState = GAME_STATE_OPTIONS_MAIN;
	setOptionPageLine(renderer, &optionPage_Game, 0, "Block Frequency", 5, 1, STAY, true);
#if defined(THREEDS) || defined(PSP) || defined(WII_U) || defined(VITA)
	setOptionChoice(renderer,   &optionPage_Game, 0, 0, "Low", "Change the number of obstacles.", "NOTE: High frequency = worse framerate", EMPTY);
#else
	setOptionChoice(renderer,   &optionPage_Game, 0, 0, "Low", "Change the number of obstacles.", EMPTY, EMPTY);
#endif
	setOptionChoice(renderer,   &optionPage_Game, 0, 1, "Medium", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 0, 2, "High", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 0, 3, "Very High", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 0, 4, "Intense", EMPTY, EMPTY, EMPTY);
#if defined(THREEDS) || defined(PSP) || defined(WII_U) || defined(VITA)
	setOptionPageLine(renderer, &optionPage_Game, 1, "Block Size", 4, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Game, 1, 0, "Normal", "Change the size of the", "incoming obstacles.", "NOTE: Large size = worse framerate");
#else
	setOptionPageLine(renderer, &optionPage_Game, 1, "Block Size", 4, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Game, 1, 0, "Normal", "Change the size of the", "incoming obstacles.", EMPTY);
#endif
	setOptionChoice(renderer,   &optionPage_Game, 1, 1, "Large", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 1, 2, "Very Large", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 1, 3, "Giant", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Game, 2, "Lives", 3, 2, STAY, true);
	setOptionChoice(renderer,   &optionPage_Game, 2, 0, "1", "Change how many hits you can take.", EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 2, 1, "2", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 2, 2, "3", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Game, 3, "Stick Controls", 2, 1, STAY, false);
	setOptionChoice(renderer,   &optionPage_Game, 3, 0, "Cardinal", "Up/Down and Left/Right movement", "are independent, so diagonal is", "faster.");
	setOptionChoice(renderer,   &optionPage_Game, 3, 1, "True Analog", "Speed is the same", "regardless of direction.", "More analog stick-friendly.");

	optionPage_Visual.pageID = 3;
	optionPage_Visual.numLines = OPTION_PAGE_VISUAL_NUM_LINES;
	optionPage_Visual.optionLines = optionPage_Visual_Lines;
	optionPage_Visual.prevState = GAME_STATE_OPTIONS_MAIN;
	if (compactView) {
		setOptionPageLine(renderer, &optionPage_Visual, 0, "BG Color", 5, 0, STAY, true);
	} else {
		setOptionPageLine(renderer, &optionPage_Visual, 0, "Background Color", 5, 0, STAY, true);
	}
	setOptionChoice(renderer,   &optionPage_Visual, 0, 0, "Electric Green", "Change the background color.", EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 0, 1, "Ocean Blue", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 0, 2, "Space Blue", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 0, 3, "Lava Red", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 0, 4, "Void Black", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Visual, 1, "Block Color", 5, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Visual, 1, 0, "Lightning", "Change the color of obstacles.", EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 1, 1, "Plant", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 1, 2, "Charcoal", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 1, 3, "Snow", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 1, 4, "Fire", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Visual, 2, "Overlay Color", 7, 5, STAY, true);
	setOptionChoice(renderer,   &optionPage_Visual, 2, 0, "Electric Green", "Change the overlay color", "on non-square displays.", EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 2, 1, "Ocean Blue", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 2, 2, "Space Blue", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 2, 3, "Lava Red", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 2, 4, "Void Black", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 2, 5, "Pitch Black", EMPTY, EMPTY, EMPTY);
	if (compactView) {
		setOptionChoice(renderer, &optionPage_Visual, 2, 6, "Match BG", EMPTY, EMPTY, EMPTY);
	} else {
		setOptionChoice(renderer, &optionPage_Visual, 2, 6, "Match Background", EMPTY, EMPTY, EMPTY);
	}
	if (compactView) {
		setOptionPageLine(renderer, &optionPage_Visual, 3, "Transparency", 2, 0, STAY, true);
	} else {
		setOptionPageLine(renderer, &optionPage_Visual, 3, "Block Transparency", 2, 0, STAY, true);
	}
	setOptionChoice(renderer,   &optionPage_Visual, 3, 0, "On", "Toggle the fade-in transparency", "effect on distant blocks.", EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 3, 1, "Off", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Visual, 4, "Speedometer", 2, 1, STAY, true);
	setOptionChoice(renderer,   &optionPage_Visual, 4, 0, "Off", "Show your speed in the", "bottom-right corner of the screen.", EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 4, 1, "On", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Visual, 5, "Frame Rate Cap", 5, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Visual, 5, 0, "Uncapped", "Set the max frame rate.",
#if defined(WII) || defined(SWITCH) || defined(PC)
		"(Though you can just keep it", "uncapped on this system)");
#else
		"Useful if you want to", "prioritize consistent visuals.");
#endif
	setOptionChoice(renderer,   &optionPage_Visual, 5, 1, "30 FPS", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 5, 2, "40 FPS", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 5, 3, "50 FPS", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 5, 4, "60 FPS", EMPTY, EMPTY, EMPTY);
#if defined(PC)
	setOptionPageLine(renderer, &optionPage_Visual, 6, "Fullscreen", 2, 0, STAY, true);
	setOptionChoice(renderer,   &optionPage_Visual, 6, 0, "Off", "Display the game in fullscreen.", EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Visual, 6, 1, "On", EMPTY, EMPTY, EMPTY);
#endif

	optionPage_Audio.pageID = 4;
	optionPage_Audio.numLines = OPTION_PAGE_AUDIO_NUM_LINES;
	optionPage_Audio.optionLines = optionPage_Audio_Lines;
	optionPage_Audio.prevState = GAME_STATE_OPTIONS_MAIN;
	setOptionPageLine(renderer, &optionPage_Audio, 0, "Music", 5, 0, STAY, false);
	setOptionChoice(renderer,   &optionPage_Audio, 0, 0, "#1", "Raina ft. Coaxcable", "\"Spaceranger 50k\"", EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 0, 1, "#2", "Cobburn and Monty", "\"Falling Up\"", EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 0, 2, "#3", "Diomatic", "\"Falling People\"", EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 0, 3, "#4", "mano and ske", "\"Darkness in da Night\"", EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 0, 4, "#5", "Diablo", "\"Dance 2 Insanity\"", EMPTY);
	setOptionPageLine(renderer, &optionPage_Audio, 1, "Music Volume", 6, 5, STAY, true);
	setOptionChoice(renderer,   &optionPage_Audio, 1, 0, "0", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 1, 1, "1", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 1, 2, "2", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 1, 3, "3", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 1, 4, "4", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 1, 5, "5", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Audio, 2, "SFX Volume", 6, 5, STAY, true);
	setOptionChoice(renderer,   &optionPage_Audio, 2, 0, "0", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 2, 1, "1", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 2, 2, "2", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 2, 3, "3", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 2, 4, "4", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Audio, 2, 5, "5", EMPTY, EMPTY, EMPTY);
}

void openPage(SDL_Renderer *renderer, OptionPage *page, bool resetIndex) {
	if (resetIndex) {
		page->index = 0;
	}
	setMessagePosRelativeToScreen_LeftAlign(&message_menu_cursor, CURSOR_X, CURSOR_Y);
}

void optionCallback_CubeFrequency(SDL_Renderer *renderer) {
	switch (OPTION_CUBE_FREQUENCY) {
	case 0:
		cubeAmount = (Sint16)(400 * CUBE_LIMIT_MULT);
		break;
	case 1:
		cubeAmount = (Sint16)(500 * CUBE_LIMIT_MULT);
		break;
	case 2:
		cubeAmount = (Sint16)(600 * CUBE_LIMIT_MULT);
		break;
	case 3:
		cubeAmount = (Sint16)(700 * CUBE_LIMIT_MULT);
		break;
	default:
		cubeAmount = CUBE_LIMIT_MAX;
		break;
	}
	prepareGame();
}

static void optionCallback_CubeSize() {
	// Approximate formula: y = -0.512x^2 + 0.7616x + 0.3422
	switch (OPTION_CUBE_SIZE) {
	case 0:
		cubeSize = 0.5f;
		cubeSizeHalf = 0.25f;
		cubeSizeLimit = 0.5f; // from formula: 0.5006 (should definitely be 0.5)
		break;
	case 1:
		cubeSize = 0.625f;
		cubeSizeHalf = 0.3125f;
		cubeSizeLimit = 0.532f; // from formula: 0.5302
		break;
	case 2:
		cubeSize = 0.75f;
		cubeSizeHalf = 0.375f;
		cubeSizeLimit = 0.554f; // from formula: 0.5558
		break;
	default:
		cubeSize = 0.875f;
		cubeSizeHalf = 0.4375f;
		cubeSizeLimit = 0.578f; // from formula 0.5774
		break;
	}
	prepareGame();
}

void optionCallback_OverlayColor() {
	switch (OPTION_OVERLAY_COLOR) {
	case 0:
		overlayColor = (SDL_Color){ .r = 15, .g = 255, .b = 155 };
		break;
	case 1:
		overlayColor = (SDL_Color){ .r = 0, .g = 45, .b = 255 };
		break;
	case 2:
		overlayColor = (SDL_Color){ .r = 15, .g = 0, .b = 155 };
		break;
	case 3:
		overlayColor = (SDL_Color){ .r = 200, .g = 35, .b = 35 };
		break;
	case 4:
		overlayColor = (SDL_Color){ .r = 15, .g = 15, .b = 15 };
		break;
	case 5:
		overlayColor = (SDL_Color){ .r = 0, .g = 0, .b = 0 };
		break;
	default:
		overlayColor = backgroundColor;
		break;
	}
	drawOverlayOnThisFrame = true;
}

static void optionCallback_BackgroundColor() {
	switch (OPTION_BACKGROUND_COLOR) {
	case 0:
		backgroundColor = (SDL_Color){ .r = 15, .g = 255, .b = 155 };
		break;
	case 1:
		backgroundColor = (SDL_Color){ .r = 0, .g = 45, .b = 255 };
		break;
	case 2:
		backgroundColor = (SDL_Color){ .r = 15, .g = 0, .b = 155 };
		break;
	case 3:
		backgroundColor = (SDL_Color){ .r = 200, .g = 35, .b = 35 };
		break;
	default:
		backgroundColor = (SDL_Color){ .r = 15, .g = 15, .b = 15 };
		break;
	}
	optionCallback_OverlayColor();
}

static void optionCallback_CubeColor() {
	switch (OPTION_CUBE_COLOR) {
	case 0:
		cubeColorFront = (SDL_Color){ .r = 200, .g = 250, .b = 120, .a = 255 };
		cubeColorSide = (SDL_Color){ .r = 100, .g = 100, .b = 200, .a = 255 };
		break;
	case 1:
		cubeColorFront = (SDL_Color){ .r = 0, .g = 255, .b = 150, .a = 255 };
		cubeColorSide = (SDL_Color){ .r = 50, .g = 50, .b = 50, .a = 255 };
		break;
	case 2:
		cubeColorFront = (SDL_Color){ .r = 255, .g = 100, .b = 100, .a = 255 };
		cubeColorSide = (SDL_Color){ .r = 70, .g = 90, .b = 80, .a = 255 };
		break;
	case 3:
		cubeColorFront = (SDL_Color){ .r = 170, .g = 195, .b = 255, .a = 255 };
		cubeColorSide = (SDL_Color){ .r = 120, .g = 135, .b = 180, .a = 255 };
		break;
	default:
		cubeColorFront = (SDL_Color){ .r = 255, .g = 150, .b = 0, .a = 255 };
		cubeColorSide = (SDL_Color){ .r = 180, .g = 105, .b = 0, .a = 255 };
		break;
	}
}

void optionCallback_FrameRate() {
	switch (OPTION_FRAME_RATE) {
	case 0:
		frameRate = (displayRefreshRate < 255) ? displayRefreshRate : 255;
		break;
	case 1:
		frameRate = 30;
		break;
	case 2:
		frameRate = 40;
		break;
	case 3:
		frameRate = 50;
		break;
	default:
		frameRate = 60;
		break;
	}
	ticksPerFrame = (Uint32)(1000 / frameRate);
}

void optionCallback_Fullscreen(SDL_Window *window, OptionPage *page) {
#if defined(PC)
	SDL_SetWindowFullscreen(window, OPTION_FULLSCREEN * SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
}

static void optionCallback_Music() {
	playMusicAtIndex(OPTION_MUSIC);
}

static void optionCallback_MusicVolume() {
	Mix_VolumeMusic((int)(OPTION_MUSIC_VOLUME * MIX_MAX_VOLUME / 5.0f));
}

static void optionCallback_SFXVolume() {
	Mix_Volume(-1, (int)(OPTION_SFX_VOLUME * MIX_MAX_VOLUME / 5.0f));
}

void optionCallback_All() {
	// Fullscreen and Music callbacks are omitted
	optionCallback_CubeFrequency(renderer);
	optionCallback_CubeSize();
	optionCallback_BackgroundColor();
	saveBackgroundAsTexture(renderer);
	optionCallback_CubeColor();
	optionCallback_OverlayColor();
	optionCallback_FrameRate();
	optionCallback_MusicVolume();
	optionCallback_SFXVolume();
}

// Handle specific callbacks here
static void optionCallback(SDL_Window *window, OptionPage *page) {
	switch (page->pageID) {
	case 2:
		switch (page->index) {
		case 0:
			optionCallback_CubeFrequency(renderer);
			break;
		case 1:
			optionCallback_CubeSize();
			break;
		default:
			break;
		}
		break;
	case 3:
		switch (page->index) {
		case 0:
			optionCallback_BackgroundColor();
			saveBackgroundAsTexture(renderer);
			break;
		case 1:
			optionCallback_CubeColor();
			break;
		case 2:
			optionCallback_OverlayColor();
			break;
		case 5:
			optionCallback_FrameRate();
			break;
		case 6:
			optionCallback_Fullscreen(window, page);
			break;
		default:
			break;
		}
		break;
	case 4:
		switch (page->index) {
		case 0:
			optionCallback_Music();
			break;
		case 1:
			optionCallback_MusicVolume();
			break;
		case 2:
			optionCallback_SFXVolume();
			playSFX(SFX_DING_A);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

static inline void drawOptionTextFromChars(SDL_Renderer *renderer, Message message_characters[], char text[], Uint8 numChars, int startingX, float relY) {
#if defined(PSP)
	startingX += 104;
#endif
	// Render all outlines, then all text
	for (int i = 0; i < numChars; i++) {
		Uint8 chr = text[i] - FIRST_PRINTABLE_CHAR;
		message_characters[chr].text_rect.x = startingX + i * message_characters[chr].text_rect.w;
		message_characters[chr].outline_rect.x = message_characters[chr].text_rect.x - (message_characters[chr].outline_rect.w - message_characters[chr].text_rect.w) / 2;
		setMessagePosRelativeToScreenY(&message_characters[chr], relY);
		SDL_RenderCopy(renderer, message_characters[chr].outline_texture, NULL, &message_characters[chr].outline_rect);
	}
	for (int i = 0; i < numChars; i++) {
		Uint8 chr = text[i] - FIRST_PRINTABLE_CHAR;
		message_characters[chr].text_rect.x = startingX + i * message_characters[chr].text_rect.w;
		SDL_RenderCopy(renderer, message_characters[chr].text_texture, NULL, &message_characters[chr].text_rect);
	}
}

static inline void drawLineFromChars(SDL_Renderer *renderer, OptionLine *line, OptionChoice *choice) {
	Uint8 numChars = (Uint8)strlen(line->name);
	drawOptionTextFromChars(renderer, message_characters_white_38, line->name, numChars, (int)(LINE_X * GAME_WIDTH), line->y);
	numChars = (Uint8)strlen(choice->name);
	drawOptionTextFromChars(renderer, message_characters_gray_38, choice->name, numChars, (int)(CHOICE_X * GAME_WIDTH), choice->y);
}

static inline void drawDescFromChars(SDL_Renderer *renderer, OptionChoice *choice) {
	Uint8 numChars;
	int startingX;
	Sint8 numDescLines = 0;
	if (strcmp(choice->descLine1, EMPTY) != 0) numDescLines++;
	if (strcmp(choice->descLine2, EMPTY) != 0) numDescLines++;
	if (strcmp(choice->descLine3, EMPTY) != 0) numDescLines++;
	switch (numDescLines) {
	case 1:
		numChars = (Uint8)strlen(choice->descLine1);
		startingX = (int)(0.5f * GAME_WIDTH - (numChars * message_characters_gray_38[0].text_rect.w) / 2);
		drawOptionTextFromChars(renderer, message_characters_white_38, choice->descLine1, numChars, startingX, DESC_LINE_Y_POS_3);
		break;
	case 2:
		numChars = (Uint8)strlen(choice->descLine1);
		startingX = (int)(0.5f * GAME_WIDTH - (numChars * message_characters_gray_38[0].text_rect.w) / 2);
		drawOptionTextFromChars(renderer, message_characters_white_38, choice->descLine1, numChars, startingX, DESC_LINE_Y_POS_2);
		numChars = (Uint8)strlen(choice->descLine2);
		startingX = (int)(0.5f * GAME_WIDTH - (numChars * message_characters_gray_38[0].text_rect.w) / 2);
		drawOptionTextFromChars(renderer, message_characters_white_38, choice->descLine2, numChars, startingX, DESC_LINE_Y_POS_4);
		break;
	default:
		numChars = (Uint8)strlen(choice->descLine1);
		startingX = (int)(0.5f * GAME_WIDTH - (numChars * message_characters_gray_38[0].text_rect.w) / 2);
		drawOptionTextFromChars(renderer, message_characters_white_38, choice->descLine1, numChars, startingX, DESC_LINE_Y_POS_1);
		numChars = (Uint8)strlen(choice->descLine2);
		startingX = (int)(0.5f * GAME_WIDTH - (numChars * message_characters_gray_38[0].text_rect.w) / 2);
		drawOptionTextFromChars(renderer, message_characters_white_38, choice->descLine2, numChars, startingX, DESC_LINE_Y_POS_3);
		numChars = (Uint8)strlen(choice->descLine3);
		startingX = (int)(0.5f * GAME_WIDTH - (numChars * message_characters_gray_38[0].text_rect.w) / 2);
		drawOptionTextFromChars(renderer, message_characters_white_38, choice->descLine3, numChars, startingX, DESC_LINE_Y_POS_5);
		break;
	}
}

void handlePage(SDL_Renderer *renderer, SDL_Window *window, OptionPage *page, bool renderCursor) {
	if (page->pageID != 3) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 64);
#if defined(PSP)
		SDL_Rect rect = { 0, 0, GAME_WIDTH + 104, GAME_HEIGHT };
#else
		SDL_Rect rect = { 0, 0, GAME_WIDTH, GAME_HEIGHT };
#endif
		SDL_RenderFillRect(renderer, &rect);
	}

	// Handle Up/Down input
	if (page->numLines > 1) {
		if (dirPressedUp()) {
			page->index = (page->index - 1) % page->numLines;
			if (page->index < 0) {
				page->index = page->numLines - 1;
			}
		} else if (dirPressedDown()) {
			page->index = (page->index + 1) % page->numLines;
		}
	}

	// Handle Left/Right input
	bool optionChanged = false;
	if (dirPressedLeft()) {
		OptionLine *currentLine = &page->optionLines[page->index];
		currentLine->index = (currentLine->index - 1 + currentLine->numChoices) % currentLine->numChoices;
		optionCallback(window, page);
	} else if (dirPressedRight()) {
		OptionLine *currentLine = &page->optionLines[page->index];
		currentLine->index = (currentLine->index + 1) % currentLine->numChoices;
		optionCallback(window, page);
	}

	// Handle Confirm press
	if (keyPressed(INPUT_A) || keyPressed(INPUT_START)) {
		optionCallback(window, page);
		if (page->optionLines[page->index].nextState != STAY) {
			gameState = page->optionLines[page->index].nextState;
			switch (gameState) {
				case GAME_STATE_OPTIONS_GAME:
					openPage(renderer, &optionPage_Game, true);
					break;
				case GAME_STATE_OPTIONS_VISUAL:
					openPage(renderer, &optionPage_Visual, true);
					break;
				case GAME_STATE_OPTIONS_AUDIO:
					openPage(renderer, &optionPage_Audio, true);
					break;
				default:
					break;
			}
		}
	// Handle Back press
	} else if (keyPressed(INPUT_B) || keyPressed(INPUT_SELECT)) {
		gameState = page->prevState;
		switch (gameState) {
			case GAME_STATE_OPTIONS_MAIN:
				openPage(renderer, &optionPage_Main, false);
				break;
			case GAME_STATE_OPTIONS_GAME:
				openPage(renderer, &optionPage_Game, false);
				break;
			case GAME_STATE_TITLE_SCREEN:
				writeSaveData();
				break;
			default:
				break;
		}
	}
	setMessagePosRelativeToScreen_LeftAlign(&message_menu_cursor, CURSOR_X, CURSOR_Y);

	if (renderCursor) {
		renderMessage(renderer, &message_menu_cursor);
	}
	for (Sint8 lineIndex = 0; lineIndex < page->numLines; lineIndex++) {
		OptionLine *currLine = &page->optionLines[lineIndex];
		OptionChoice *currChoice = &currLine->optionChoices[currLine->index];
		drawLineFromChars(renderer, currLine, currChoice);
	}
	OptionChoice *currChoice = &page->optionLines[page->index]
		.optionChoices[page->optionLines[page->index].oneDesc ? 0 : page->optionLines[page->index].index];
	drawDescFromChars(renderer, currChoice);
}

static void freePage(OptionPage *page) {
	if (page == NULL) return;
	for (int i = 0; i < page->numLines; i++) {
		free(page->optionLines[i].optionChoices);
	}
}

void cleanUpMenu() {
	freePage(&optionPage_Empty);
	freePage(&optionPage_Main);
	freePage(&optionPage_Game);
	freePage(&optionPage_Visual);
	freePage(&optionPage_Audio);
}