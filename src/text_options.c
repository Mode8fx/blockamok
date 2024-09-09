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

OptionPage optionPage_Empty;
OptionLine optionPage_Empty_Lines[1];

#define CURR_LINE page->optionLines[lineIndex]

static void setOptionPageLine(SDL_Renderer *renderer, OptionPage *page, int lineIndex, char text[], Sint8 numChoices, Sint8 choiceIndex, int nextState) {
	snprintf(CURR_LINE.name.text, TEXT_LINE_SIZE, text);
	CURR_LINE.numChoices = numChoices;
	CURR_LINE.index = choiceIndex;
	CURR_LINE.optionChoices = (OptionChoice *)malloc(numChoices * sizeof(OptionChoice));
	for (int i = 0; i < numChoices; i++) {
		CURR_LINE.optionChoices[i] = (OptionChoice){0};
	}
	CURR_LINE.nextState = nextState;
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &CURR_LINE.name, 1, color_white, color_black);
	setMessagePosRelativeToScreen_LeftAlign(&CURR_LINE.name, LINE_X, LINE_Y);
}

static void setOptionChoice(SDL_Renderer *renderer, OptionPage *page, int lineIndex, int choiceIndex, char name[], Sint8 numDescLines, char descLine1[], char descLine2[], char descLine3[]) {
	OptionChoice *currChoice = &CURR_LINE.optionChoices[choiceIndex];
	snprintf(currChoice->name.text, TEXT_LINE_SIZE, "%s", name);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->name, 1, color_white, color_black);
	setMessagePosRelativeToScreen_LeftAlign(&currChoice->name, CHOICE_X, CHOICE_Y);
	snprintf(currChoice->descLine1.text, TEXT_LINE_SIZE, "%s", descLine1);
	snprintf(currChoice->descLine2.text, TEXT_LINE_SIZE, "%s", descLine2);
	snprintf(currChoice->descLine3.text, TEXT_LINE_SIZE, "%s", descLine3);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->descLine1, 1, color_gray, color_black);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->descLine2, 1, color_gray, color_black);
	prepareMessage(renderer, OPTION_FONT, OPTION_OUTLINE_SIZE, &currChoice->descLine3, 1, color_gray, color_black);
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
	setOptionPageLine(renderer, &optionPage_Main, 0, "Game Options", 2, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Main, 0, 0, "TEST ABC", 3, "TEST A", "TEST B", "TEST C");
	setOptionChoice(renderer, &optionPage_Main, 0, 1, "TEST DEF", 3, "TEST D", "TEST E", "TEST F");
	setOptionPageLine(renderer, &optionPage_Main, 1, "Visuals", 1, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Main, 1, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 2, "Audio", 1, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Main, 2, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 3, "Instructions", 1, 0, GAME_STATE_INSTRUCTIONS);
	setOptionChoice(renderer, &optionPage_Main, 3, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 4, "Credits", 1, 0, GAME_STATE_CREDITS);
	setOptionChoice(renderer, &optionPage_Main, 4, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 5, "Reset High Score", 1, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Main, 5, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 6, "Reset Settings", 1, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Main, 6, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
	setOptionPageLine(renderer, &optionPage_Main, 7, "Quit", 1, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Main, 7, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);

	optionPage_Empty.optionLines = optionPage_Empty_Lines;
	optionPage_Empty.prevState = GAME_STATE_OPTIONS;
	setOptionPageLine(renderer, &optionPage_Empty, 0, EMPTY, 1, 0, GAME_STATE_OPTIONS);
	setOptionChoice(renderer, &optionPage_Empty, 0, 0, EMPTY, 1, EMPTY, EMPTY, EMPTY);
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
		if (page->optionLines[page->index].nextState != -1) {
			gameState = page->optionLines[page->index].nextState;
		}
	} else if (keyPressed(INPUT_B)) {
		gameState = page->prevState;
		switch (gameState) {
			case GAME_STATE_OPTIONS:
				openPage(renderer, &optionPage_Main, false);
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
		renderMessage(renderer, &CURR_LINE.name);
		renderMessage(renderer, &CURR_LINE.optionChoices[CURR_LINE.index].name);
	}
	OptionChoice *currChoice = &page->optionLines[page->index].optionChoices[page->optionLines[page->index].index];
	renderMessage(renderer, &currChoice->descLine1);
	renderMessage(renderer, &currChoice->descLine2);
	renderMessage(renderer, &currChoice->descLine3);
}