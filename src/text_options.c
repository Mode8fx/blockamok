#include "./text.h"
#include "./game.h"
#include "./input.h"
#include "./config.h"

#include <stdio.h>

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
#define OPTION_PAGE_MAIN_NUM_LINES 8
OptionLine optionPage_Main_Lines[OPTION_PAGE_MAIN_NUM_LINES];

OptionPage optionPage_Game;
#define OPTION_PAGE_GAME_NUM_LINES 3
OptionLine optionPage_Game_Lines[OPTION_PAGE_GAME_NUM_LINES];

OptionPage optionPage_Empty;
OptionLine optionPage_Empty_Lines[1];
#define STAY -1

static void setOptionPageLine(SDL_Renderer *renderer, OptionPage *page, int lineIndex, char text[], Sint8 numChoices, Sint8 choiceIndex, int nextState, bool oneDesc) {
	OptionLine *currLine = &page->optionLines[lineIndex];
	snprintf(currLine->name.text, TEXT_LINE_SIZE, text);
	currLine->numChoices = numChoices;
	currLine->index = choiceIndex;
	currLine->optionChoices = (OptionChoice *)malloc(numChoices * sizeof(OptionChoice));
	for (int i = 0; i < numChoices; i++) {
		currLine->optionChoices[i] = (OptionChoice){ 0 };
	}
	currLine->nextState = nextState;
	currLine->oneDesc = oneDesc;
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currLine->name, 1, color_white, color_black);
	setMessagePosRelativeToScreen_LeftAlign(&currLine->name, LINE_X, LINE_Y);
}

static void setOptionChoice(SDL_Renderer *renderer, OptionPage *page, int lineIndex, int choiceIndex, char name[], char descLine1[], char descLine2[], char descLine3[]) {
	OptionChoice *currChoice = &page->optionLines[lineIndex].optionChoices[choiceIndex];
	snprintf(currChoice->name.text, TEXT_LINE_SIZE, "%s", name);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->name, 1, color_gray, color_black);
	setMessagePosRelativeToScreen_LeftAlign(&currChoice->name, CHOICE_X, CHOICE_Y);
	snprintf(currChoice->descLine1.text, TEXT_LINE_SIZE, "%s", descLine1);
	snprintf(currChoice->descLine2.text, TEXT_LINE_SIZE, "%s", descLine2);
	snprintf(currChoice->descLine3.text, TEXT_LINE_SIZE, "%s", descLine3);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->descLine1, 1, color_gray, color_black);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->descLine2, 1, color_gray, color_black);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->descLine3, 1, color_gray, color_black);

	Sint8 numDescLines = 0;
	if (strcmp(descLine1, EMPTY) != 0) numDescLines++;
	if (strcmp(descLine2, EMPTY) != 0) numDescLines++;
	if (strcmp(descLine3, EMPTY) != 0) numDescLines++;
	switch (numDescLines) {
		case 2:
			setMessagePosRelativeToScreen(&currChoice->descLine1, 0.5f, DESC_LINE_Y_POS_2);
			setMessagePosRelativeToScreen(&currChoice->descLine2, 0.5f, DESC_LINE_Y_POS_4);
			setMessagePosRelativeToScreen(&currChoice->descLine3, 0.5f, 0);
			break;
		case 3:
			setMessagePosRelativeToScreen(&currChoice->descLine1, 0.5f, DESC_LINE_Y_POS_1);
			setMessagePosRelativeToScreen(&currChoice->descLine2, 0.5f, DESC_LINE_Y_POS_3);
			setMessagePosRelativeToScreen(&currChoice->descLine3, 0.5f, DESC_LINE_Y_POS_5);
			break;
		default:
			setMessagePosRelativeToScreen(&currChoice->descLine1, 0.5f, DESC_LINE_Y_POS_3);
			setMessagePosRelativeToScreen(&currChoice->descLine2, 0.5f, 0);
			setMessagePosRelativeToScreen(&currChoice->descLine3, 0.5f, 0);
			break;
		}
}

void initStaticMessages_Options(SDL_Renderer *renderer) {
	snprintf(message_menu_cursor.text, TEXT_LINE_SIZE, ">");
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &message_menu_cursor, 1, color_white, color_black);

	optionPage_Main.numLines = OPTION_PAGE_MAIN_NUM_LINES;
	optionPage_Main.optionLines = optionPage_Main_Lines;
	optionPage_Main.prevState = GAME_STATE_TITLE_SCREEN;
	setOptionPageLine(renderer, &optionPage_Main, 0, "Game Options", 1, 0, GAME_STATE_OPTIONS_GAME, true);
	setOptionChoice(renderer,   &optionPage_Main, 0, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 1, "Visuals", 1, 0, GAME_STATE_OPTIONS_MAIN, true);
	setOptionChoice(renderer,   &optionPage_Main, 1, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 2, "Audio", 1, 0, GAME_STATE_OPTIONS_MAIN, true);
	setOptionChoice(renderer,   &optionPage_Main, 2, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 3, "Instructions", 1, 0, GAME_STATE_INSTRUCTIONS, true);
	setOptionChoice(renderer,   &optionPage_Main, 3, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 4, "Credits", 1, 0, GAME_STATE_CREDITS, true);
	setOptionChoice(renderer,   &optionPage_Main, 4, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 5, "Reset High Score", 1, 0, GAME_STATE_OPTIONS_MAIN, true);
	setOptionChoice(renderer,   &optionPage_Main, 5, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 6, "Reset Settings", 1, 0, GAME_STATE_OPTIONS_MAIN, true);
	setOptionChoice(renderer,   &optionPage_Main, 6, 0, EMPTY, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 7, "Quit", 1, 0, GAME_STATE_OPTIONS_MAIN, true);
	setOptionChoice(renderer,   &optionPage_Main, 7, 0, EMPTY, EMPTY, EMPTY, EMPTY);

	optionPage_Empty.optionLines = optionPage_Empty_Lines;
	optionPage_Empty.prevState = GAME_STATE_OPTIONS_MAIN;
	setOptionPageLine(renderer, &optionPage_Empty, 0, EMPTY, 1, 0, GAME_STATE_OPTIONS_MAIN, true);
	setOptionChoice(renderer,   &optionPage_Empty, 0, 0, EMPTY, EMPTY, EMPTY, EMPTY);

	optionPage_Game.numLines = OPTION_PAGE_GAME_NUM_LINES;
	optionPage_Game.optionLines = optionPage_Game_Lines;
	optionPage_Game.prevState = GAME_STATE_OPTIONS_MAIN;
	setOptionPageLine(renderer, &optionPage_Game, 0, "Cube Amount", 4, 1, STAY, true);
	setOptionChoice(renderer,   &optionPage_Game, 0, 0, "Small", "Change the number of obstacles.", EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 0, 1, "Normal", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 0, 2, "Large", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 0, 3, "Very Large", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Game, 1, "Cube Size", 4, 1, STAY, true);
	setOptionChoice(renderer,   &optionPage_Game, 1, 0, "Small", "Change the size of the", "incoming obstacles.", EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 1, 1, "Normal", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 1, 2, "Large", EMPTY, EMPTY, EMPTY);
	setOptionChoice(renderer,   &optionPage_Game, 1, 3, "Very Large", EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Game, 2, "Control Scheme", 2, 0, STAY, false);
	setOptionChoice(renderer,   &optionPage_Game, 2, 0, "Type A", "Up/Down and Left/Right movement", "are independent, so diagonal is", "faster.");
	setOptionChoice(renderer,   &optionPage_Game, 2, 1, "Type B", "Speed is the same", "regardless of direction.", "More analog stick-friendly.");
}

void openPage(SDL_Renderer *renderer, OptionPage *page, bool resetIndex) {
	if (resetIndex) {
		page->index = 0;
	}
	setMessagePosRelativeToScreen_LeftAlign(&message_menu_cursor, CURSOR_X, CURSOR_Y);
}

void handlePage(SDL_Renderer *renderer, OptionPage *page, bool renderCursor) {
	if (page->numLines > 1) {
		if (keyPressed(INPUT_UP)) {
			page->index = (page->index - 1) % page->numLines;
			if (page->index < 0) {
				page->index = page->numLines - 1;
			}
			setMessagePosRelativeToScreen_LeftAlign(&message_menu_cursor, CURSOR_X, CURSOR_Y);
		} else if (keyPressed(INPUT_DOWN)) {
			page->index = (page->index + 1) % page->numLines;
			setMessagePosRelativeToScreen_LeftAlign(&message_menu_cursor, CURSOR_X, CURSOR_Y);
		}
	}
	if (keyPressed(INPUT_LEFT)) {
		OptionLine *currentLine = &page->optionLines[page->index];
		currentLine->index = (currentLine->index - 1 + currentLine->numChoices) % currentLine->numChoices;
	} else if (keyPressed(INPUT_RIGHT)) {
		OptionLine *currentLine = &page->optionLines[page->index];
		currentLine->index = (currentLine->index + 1) % currentLine->numChoices;
	}
	if (keyPressed(INPUT_A)) {
		if (page->optionLines[page->index].nextState != STAY) {
			gameState = page->optionLines[page->index].nextState;
		}
	} else if (keyPressed(INPUT_B)) {
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


	if (renderCursor) {
		renderMessage(renderer, &message_menu_cursor);
	}
	for (Sint8 lineIndex = 0; lineIndex < page->numLines; lineIndex++) {
		OptionLine *currLine = &page->optionLines[lineIndex];
		renderMessage(renderer, &currLine->name);
		renderMessage(renderer, &currLine->optionChoices[currLine->index].name);
	}
	OptionChoice *currChoice = &page->optionLines[page->index]
		.optionChoices[page->optionLines[page->index].oneDesc ? 0 : page->optionLines[page->index].index];
	renderMessage(renderer, &currChoice->descLine1);
	renderMessage(renderer, &currChoice->descLine2);
	renderMessage(renderer, &currChoice->descLine3);
}