#include "./archipelago.h"
#include "./input.h"
#include "./game.h"

bool ap_isEnabled = false;
#define AP_SCORE_THRESHOLD 5000
int ap_pointsUntilHint = AP_SCORE_THRESHOLD;
char ap_room[5] = "#####";

void ap_handleHintCheck() {
	// TODO: Implement Archipelago hint-giving here
	ap_pointsUntilHint = AP_SCORE_THRESHOLD;
}

void ap_handleInputCheck() {
	if (keyHeld(INPUT_L) && keyHeld(INPUT_R) && keyHeld(INPUT_A) && keyHeld(INPUT_RIGHT)) {
		gameState = GAME_STATE_ARCHIPELAGO_LOGIN;
	}
}

void ap_handleLoginScreen() {
	// TODO: Handle room number input here
	if (keyPressed(INPUT_B)) {
		gameState = GAME_STATE_TITLE_SCREEN;
	} else if (keyPressed(INPUT_A)) {
		ap_enableArchipelago();
		gameState = GAME_STATE_TITLE_SCREEN;
	}
}

void ap_enableArchipelago() {
	// TODO: Handle AP connection here
	ap_isEnabled = true;
	ap_pointsUntilHint = AP_SCORE_THRESHOLD;
}

void ap_handleHintScreen() {
	// TODO: Handle hint display here
}