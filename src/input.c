#if defined(WII)
#include <wiiuse/wpad.h>
#elif defined(GAMECUBE)
#include <ogc/pad.h>
#elif defined(PSP)
#include <pspctrl.h>
#endif

#include "./input.h"
#include "./game.h"

SDL_GameController *controllers[4];

Stick leftStick = {
	0, 0, 0, 0, STICK_DEADZONE, STICK_FULLZONE, STICK_THRESHOLD
};
Stick rightStick = {
	0, 0, 0, 0, STICK_DEADZONE, STICK_FULLZONE, STICK_THRESHOLD
};

Uint32 pressedKeys;
Uint32 releasedKeys;
Uint32 heldKeys;
Uint32 heldKeys_last;

double timer_buttonHold;
double timer_buttonHold_repeater;
#define REPEATER_FIRST_DELAY 500
#define REPEATER_INTERVAL 33

bool quit = false;

/////////////
// GENERAL //
/////////////

bool keyPressed(Uint32 key) {
	return (pressedKeys & key);
}

bool keyHeld(Uint32 key) {
	return (heldKeys & key);
}

//inline bool keyReleased(Uint32 key) {
//	return (releasedKeys & key);
//}

///////////
// STICK //
///////////

inline bool stickIsUp(Stick stick) {
	return stick.y < -stick.threshold;
}

inline bool stickIsDown(Stick stick) {
	return stick.y > stick.threshold;
}

inline bool stickIsLeft(Stick stick) {
	return stick.x < -stick.threshold;
}

inline bool stickIsRight(Stick stick) {
	return stick.x > stick.threshold;
}

static inline bool stickWasUp(Stick stick) {
	return stick.y_last < -stick.threshold;
}

static inline bool stickWasDown(Stick stick) {
	return stick.y_last > stick.threshold;
}

static inline bool stickWasLeft(Stick stick) {
	return stick.x_last < -stick.threshold;
}

static inline bool stickWasRight(Stick stick) {
	return stick.x_last > stick.threshold;
}

static inline bool stickPressedUp() {
	return stickIsUp(leftStick) && !stickWasUp(leftStick);
}

static inline bool stickPressedDown() {
	return stickIsDown(leftStick) && !stickWasDown(leftStick);
}

static inline bool stickPressedLeft() {
	return stickIsLeft(leftStick) && !stickWasLeft(leftStick);
}

static inline bool stickPressedRight() {
	return stickIsRight(leftStick) && !stickWasRight(leftStick);
}

static inline void updateStick(Stick *stick) {
	stick->x_last = stick->x;
	stick->y_last = stick->y;
}

static inline void applyStickZones(Stick *stick) {
	if (abs(stick->x) < stick->deadZone) {
		stick->x = 0;
	}
	else if (abs(stick->x) > stick->fullZone) {
		stick->x = (stick->x < 0) ? -32767 : 32767;
	}
	if (abs(stick->y) < stick->deadZone) {
		stick->y = 0;
	}
	else if (abs(stick->y) > stick->fullZone) {
		stick->y = (stick->y < 0) ? -32767 : 32767;
	}
}

static void mapStickToHeldKeys(Stick stick) {
	heldKeys |= (stick.x < -stick.threshold) ? INPUT_LEFT : 0;
	heldKeys |= (stick.x > stick.threshold) ? INPUT_RIGHT : 0;
	heldKeys |= (stick.y < -stick.threshold) ? INPUT_UP : 0;
	heldKeys |= (stick.y > stick.threshold) ? INPUT_DOWN : 0;
}

/////////////////////
// DIRECTION LOGIC //
/////////////////////

bool dirPressedUp() {
	return keyPressed(INPUT_UP) || stickPressedUp();
}

bool dirPressedDown() {
	return keyPressed(INPUT_DOWN) || stickPressedDown();
}

bool dirPressedLeft() {
	return keyPressed(INPUT_LEFT) || stickPressedLeft();
}

bool dirPressedRight() {
	return keyPressed(INPUT_RIGHT) || stickPressedRight();
}

bool dirHeld_Up() {
	return keyHeld(INPUT_UP) || stickIsUp(leftStick);
}

bool dirHeld_Down() {
	return keyHeld(INPUT_DOWN) || stickIsDown(leftStick);
}

bool dirHeld_Left() {
	return keyHeld(INPUT_LEFT) || stickIsLeft(leftStick);
}

bool dirHeld_Right() {
	return keyHeld(INPUT_RIGHT) || stickIsRight(leftStick);
}

///////////////////
// MAIN HANDLING //
///////////////////

static inline void updateChangedKeys() {
	pressedKeys = heldKeys & ~heldKeys_last;
	releasedKeys = ~heldKeys & heldKeys_last;
}

static inline void handleHoldTimer_prepare() {
	// If any direction is held, continue timer; otherwise, reset it
	if (heldKeys & 0x0F || leftStick.y) { // don't bother checking leftStick.x, it's not repeated anyway
		timer_buttonHold += deltaTime;
	}
	else {
		timer_buttonHold = 0;
		timer_buttonHold_repeater = 0;
	}
}

static inline void handleHoldTimer_execute() {
	// If a direction is held, "press" it again
	if (timer_buttonHold > REPEATER_FIRST_DELAY) {
		timer_buttonHold_repeater += deltaTime;
		if (timer_buttonHold_repeater >= REPEATER_INTERVAL) {
			if (dirHeld_Up()) {
				pressedKeys |= INPUT_UP;
			}
			if (dirHeld_Down()) {
				pressedKeys |= INPUT_DOWN;
			}
			// The Visual Options menu would be very seizure-inducing if this were allowed
			//if (dirHeld_Left()) {
			//	pressedKeys |= INPUT_LEFT;
			//}
			//if (dirHeld_Right()) {
			//	pressedKeys |= INPUT_RIGHT;
			//}
			timer_buttonHold_repeater -= REPEATER_INTERVAL;
		}
	}
}

void updateLastKeys() {
	heldKeys_last = heldKeys;
	updateStick(&leftStick);
	//updateStick(&rightStick);
}

void controllerInit() {
	for (Sint8 i = 0; i < 4; i++) {
		if (SDL_IsGameController(i)) {
			controllers[i] = SDL_GameControllerOpen(i);
		} else {
			controllers[i] = NULL;
		}
	}
}

///////////////////
// GAME-SPECIFIC //
///////////////////

static inline void gameSpecificInputBehavior() {
	//mapStickToHeldKeys(leftStick);
	// Map input to analog movement (where D-pad input is absolute)
	if (keyHeld(INPUT_LEFT) != keyHeld(INPUT_RIGHT)) {
		if (keyHeld(INPUT_LEFT)) {
			movementMagnitudeX = -32767;
		} else {
			movementMagnitudeX = 32767;
		}
	} else {
		movementMagnitudeX = leftStick.x;
	}
	if (keyHeld(INPUT_UP) != keyHeld(INPUT_DOWN)) {
		if (keyHeld(INPUT_UP)) {
			movementMagnitudeY = -32767;
		} else {
			movementMagnitudeY = 32767;
		}
	} else {
		movementMagnitudeY = leftStick.y;
	}
}

///////////////////
// INPUT MAPPING //
///////////////////

#if !(defined(PSP) || defined(GAMECUBE) || defined(WII))
static inline void mapInputToVar_SDL2(Uint16 varBtn, SDL_GameControllerButton inputBtn) {
	for (int i = 0; i < 4; i++) {
		if (SDL_GameControllerGetButton(controllers[i], inputBtn)) {
			heldKeys |= varBtn;
		}
	}
}
#endif

#if defined(PC) || defined(ANDROID)
static inline void mapInputToVar_Keyboard(const Uint8 *state, Uint16 varBtn, Sint32 inputBtn) {
	if (state[inputBtn]) {
		heldKeys |= varBtn;
	}
}
#endif

#if defined(PSP) || defined(WII) || defined(GAMECUBE)
static inline void mapInputToVar_GeneralConsole(Uint32 heldButtons, Uint16 varBtn, Sint32 inputBtn) {
	if (heldButtons & inputBtn) {
		heldKeys |= varBtn;
	}
}
#endif


static void handleAllCurrentInputs() {
#if !(defined(PSP) || defined(GAMECUBE) || defined(WII))
	mapInputToVar_SDL2(INPUT_UP, SDL_CONTROLLER_BUTTON_DPAD_UP);
	mapInputToVar_SDL2(INPUT_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	mapInputToVar_SDL2(INPUT_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	mapInputToVar_SDL2(INPUT_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
#if defined(SWITCH)
	mapInputToVar_SDL2(INPUT_A, SDL_CONTROLLER_BUTTON_B);
	mapInputToVar_SDL2(INPUT_B, SDL_CONTROLLER_BUTTON_A);
	mapInputToVar_SDL2(INPUT_X, SDL_CONTROLLER_BUTTON_Y);
	mapInputToVar_SDL2(INPUT_Y, SDL_CONTROLLER_BUTTON_X);
#else
	mapInputToVar_SDL2(INPUT_A, SDL_CONTROLLER_BUTTON_A);
	mapInputToVar_SDL2(INPUT_B, SDL_CONTROLLER_BUTTON_B);
	mapInputToVar_SDL2(INPUT_X, SDL_CONTROLLER_BUTTON_X);
	mapInputToVar_SDL2(INPUT_Y, SDL_CONTROLLER_BUTTON_Y);
#endif
	mapInputToVar_SDL2(INPUT_SELECT, SDL_CONTROLLER_BUTTON_BACK);
	mapInputToVar_SDL2(INPUT_START, SDL_CONTROLLER_BUTTON_START);
	mapInputToVar_SDL2(INPUT_L, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	mapInputToVar_SDL2(INPUT_R, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	mapInputToVar_SDL2(INPUT_LS, SDL_CONTROLLER_BUTTON_LEFTSTICK);
	//mapInputToVar_SDL2(INPUT_RS, SDL_CONTROLLER_BUTTON_RIGHTSTICK);

	for (int i = 0; i < 4; i++) {
		if (controllers[i]) {
			int leftTrigger = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT);
			int rightTrigger = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
			if (leftTrigger > TRIGGER_DEADZONE) {
				heldKeys |= INPUT_ZL;
			}
			if (rightTrigger > TRIGGER_DEADZONE) {
				heldKeys |= INPUT_ZR;
			}
		}
	}

	leftStick.x = 0;
	leftStick.y = 0;
	bool leftXSet = false, leftYSet = false;
	//rightStick.x = 0;
	//rightStick.y = 0;
	//bool rightXSet = false, rightYSet = false;

	for (int i = 0; i < 4; i++) {
		if (controllers[i]) {
			Stick tempLeft = leftStick;
			tempLeft.x = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_LEFTX);
			tempLeft.y = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_LEFTY);
			applyStickZones(&tempLeft);
			//Stick tempRight = rightStick;
			//tempRight.x = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_RIGHTX);
			//tempRight.y = SDL_GameControllerGetAxis(controllers[i], SDL_CONTROLLER_AXIS_RIGHTY);
			//applyStickZones(&tempRight);

			if (!leftXSet && tempLeft.x != 0) {
				leftStick.x = tempLeft.x;
				leftXSet = true;
			}
			if (!leftYSet && tempLeft.y != 0) {
				leftStick.y = tempLeft.y;
				leftYSet = true;
			}
			//if (!rightXSet && tempRight.x != 0) {
			//	rightStick.x = tempRight.x;
			//	rightXSet = true;
			//}
			//if (!rightYSet && tempRight.y != 0) {
			//	rightStick.y = tempRight.y;
			//	rightYSet = true;
			//}
			// Stop early if all axes are set
			if (leftXSet && leftYSet) {
				break;
			}
		}
	}
	applyStickZones(&leftStick);
	//applyStickZones(&rightStick);
#endif

#if defined(PC) || defined(ANDROID)
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	mapInputToVar_Keyboard(state, INPUT_UP, SDL_SCANCODE_UP);
	mapInputToVar_Keyboard(state, INPUT_UP, SDL_SCANCODE_W);
	mapInputToVar_Keyboard(state, INPUT_DOWN, SDL_SCANCODE_DOWN);
	mapInputToVar_Keyboard(state, INPUT_DOWN, SDL_SCANCODE_S);
	mapInputToVar_Keyboard(state, INPUT_LEFT, SDL_SCANCODE_LEFT);
	mapInputToVar_Keyboard(state, INPUT_LEFT, SDL_SCANCODE_A);
	mapInputToVar_Keyboard(state, INPUT_RIGHT, SDL_SCANCODE_RIGHT);
	mapInputToVar_Keyboard(state, INPUT_RIGHT, SDL_SCANCODE_D);
	mapInputToVar_Keyboard(state, INPUT_A, SDL_SCANCODE_LSHIFT);
	mapInputToVar_Keyboard(state, INPUT_A, SDL_SCANCODE_RSHIFT);
	mapInputToVar_Keyboard(state, INPUT_A, SDL_SCANCODE_KP_0);
	mapInputToVar_Keyboard(state, INPUT_B, SDL_SCANCODE_ESCAPE);
	mapInputToVar_Keyboard(state, INPUT_X, SDL_SCANCODE_X);
	mapInputToVar_Keyboard(state, INPUT_Y, SDL_SCANCODE_Y);
	mapInputToVar_Keyboard(state, INPUT_L, SDL_SCANCODE_MINUS);
	mapInputToVar_Keyboard(state, INPUT_L, SDL_SCANCODE_L);
	mapInputToVar_Keyboard(state, INPUT_R, SDL_SCANCODE_EQUALS);
	mapInputToVar_Keyboard(state, INPUT_R, SDL_SCANCODE_R);
	mapInputToVar_Keyboard(state, INPUT_RS, SDL_SCANCODE_F11);
	mapInputToVar_Keyboard(state, INPUT_START, SDL_SCANCODE_RETURN);
	mapInputToVar_Keyboard(state, INPUT_START, SDL_SCANCODE_RETURN2);
	mapInputToVar_Keyboard(state, INPUT_START, SDL_SCANCODE_KP_ENTER);
	mapInputToVar_Keyboard(state, INPUT_SELECT, SDL_SCANCODE_BACKSPACE);
#if defined(ANDROID)
	mapInputToVar_Keyboard(state, INPUT_START, SDLK_AC_BACK);
#endif
#endif

#if defined(WII) || defined(GAMECUBE)
	PAD_ScanPads();
	Uint32 heldButtonsGC = PAD_ButtonsHeld(0);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_UP, PAD_BUTTON_UP);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_DOWN, PAD_BUTTON_DOWN);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_LEFT, PAD_BUTTON_LEFT);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_RIGHT, PAD_BUTTON_RIGHT);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_A, PAD_BUTTON_A);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_B, PAD_BUTTON_B);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_X, PAD_BUTTON_X);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_Y, PAD_BUTTON_Y);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_L, PAD_TRIGGER_L);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_R, PAD_TRIGGER_R);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_START, PAD_BUTTON_START);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_SELECT, PAD_TRIGGER_Z);

	leftStick.x = PAD_StickX(0) * 256;
	leftStick.y = PAD_StickY(0) * -256;
	//rightStick.x = PAD_SubStickX(0) * 256;
	//rightStick.y = PAD_SubStickY(0) * -256;
	applyStickZones(&leftStick);
	//applyStickZones(&rightStick);
#endif

#if defined(WII)
	WPAD_ScanPads();
	Uint32 heldButtonsWii = WPAD_ButtonsHeld(0);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_UP, WPAD_BUTTON_RIGHT);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_DOWN, WPAD_BUTTON_LEFT);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_LEFT, WPAD_BUTTON_UP);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_RIGHT, WPAD_BUTTON_DOWN);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_A, WPAD_BUTTON_2);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_B, WPAD_BUTTON_1);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_X, WPAD_BUTTON_B);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_R, WPAD_BUTTON_A);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_START, WPAD_BUTTON_PLUS);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_SELECT, WPAD_BUTTON_MINUS);

	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_UP, WPAD_CLASSIC_BUTTON_UP);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_DOWN, WPAD_CLASSIC_BUTTON_DOWN);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_LEFT, WPAD_CLASSIC_BUTTON_LEFT);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_RIGHT, WPAD_CLASSIC_BUTTON_RIGHT);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_A, WPAD_CLASSIC_BUTTON_A);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_B, WPAD_CLASSIC_BUTTON_B);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_X, WPAD_CLASSIC_BUTTON_X);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_Y, WPAD_CLASSIC_BUTTON_Y);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_L, WPAD_CLASSIC_BUTTON_FULL_L);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_R, WPAD_CLASSIC_BUTTON_FULL_R);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_START, WPAD_CLASSIC_BUTTON_PLUS);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_SELECT, WPAD_CLASSIC_BUTTON_MINUS);

	// Gamecube Controller input should supercede Classic Controller
	expansion_t wii_exp;
	WPAD_Expansion(WPAD_CHAN_0, &wii_exp);
	if (wii_exp.type == EXP_CLASSIC) {
		if (leftStick.x == 0) {
			leftStick.x = ((Sint16)wii_exp.classic.ljs.pos.x - 32) * 1023;
		}
		if (leftStick.y == 0) {
			leftStick.y = ((Sint16)wii_exp.classic.ljs.pos.y - 32) * -1023;
		}
		//if (rightStick.x == 0) {
		//	rightStick.x = ((Sint16)wii_exp.classic.rjs.pos.x - 32) * 1023;
		//}
		//if (rightStick.y == 0) {
		//	rightStick.y = ((Sint16)wii_exp.classic.rjs.pos.y - 32) * -1023;
		//}
	}
	applyStickZones(&leftStick);
	//applyStickZones(&rightStick);
#endif

#if defined(PSP)
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(&pad, 1);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_UP, PSP_CTRL_UP);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_DOWN, PSP_CTRL_DOWN);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_LEFT, PSP_CTRL_LEFT);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_RIGHT, PSP_CTRL_RIGHT);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_A, PSP_CTRL_CROSS);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_B, PSP_CTRL_CIRCLE);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_X, PSP_CTRL_SQUARE);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_Y, PSP_CTRL_TRIANGLE);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_L, PSP_CTRL_LTRIGGER);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_R, PSP_CTRL_RTRIGGER);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_START, PSP_CTRL_START);
	mapInputToVar_GeneralConsole(pad.Buttons, INPUT_SELECT, PSP_CTRL_SELECT);

	leftStick.x = (pad.Lx - 128) * 256;
	leftStick.y = (pad.Ly - 128) * 256;
	applyStickZones(&leftStick);
#endif

	gameSpecificInputBehavior();
}

/////////////////
// BUTTON TEXT //
/////////////////

#if defined(PC)
	const char *btn_Up = "Up";
	const char *btn_Down = "Down";
	const char *btn_Left = "Left";
	const char *btn_Right = "Right";
	const char *btn_A = "Shift/A";
	const char *btn_B = "Esc/B";
	const char *btn_X = "X";
	const char *btn_Y = "Y";
	const char *btn_L = "-/L";
	const char *btn_R = "=/R";
	const char *btn_ZL = "?";
	const char *btn_ZR = "?";
	const char *btn_LS = "?";
	const char *btn_RS = "F11/R-Stick";
	const char *btn_Start = "Enter/Start";
	const char *btn_Select = "Back/Select";
#elif defined(GAMECUBE)
	const char *btn_Up = "Up";
	const char *btn_Down = "Down";
	const char *btn_Left = "Left";
	const char *btn_Right = "Right";
	const char *btn_A = "A";
	const char *btn_B = "B";
	const char *btn_X = "X";
	const char *btn_Y = "Y";
	const char *btn_L = "L";
	const char *btn_R = "R";
	const char *btn_ZL = "?";
	const char *btn_ZR = "?";
	const char *btn_LS = "?";
	const char *btn_RS = "?";
	const char *btn_Start = "Start";
	const char *btn_Select = "Z";
#elif defined(WII)
	const char *btn_Up = "Up";
	const char *btn_Down = "Down";
	const char *btn_Left = "Left";
	const char *btn_Right = "Right";
	const char *btn_A = "2/A";
	const char *btn_B = "1/B";
	const char *btn_X = "B (Wiimote)";
	const char *btn_Y = "X/Y";
	const char *btn_L = "A (Wiimote)";
	const char *btn_R = "L/R";
	const char *btn_ZL = "?";
	const char *btn_ZR = "?";
	const char *btn_LS = "?";
	const char *btn_RS = "?";
	const char *btn_Start = "+/Start";
	const char *btn_Select = "-/Z";
#elif defined(PSP) | defined(VITA)
	const char *btn_Up = "Up";
	const char *btn_Down = "Down";
	const char *btn_Left = "Left";
	const char *btn_Right = "Right";
	const char *btn_A = "X";
	const char *btn_B = "O";
	const char *btn_X = "Square";
	const char *btn_Y = "Triangle";
	const char *btn_L = "L";
	const char *btn_R = "R";
	const char *btn_ZL = "?";
	const char *btn_ZR = "?";
	const char *btn_LS = "?";
	const char *btn_RS = "?";
	const char *btn_Start = "Start";
	const char *btn_Select = "Select";
#elif defined(SWITCH)
	const char *btn_Up = "Up";
	const char *btn_Down = "Down";
	const char *btn_Left = "Left";
	const char *btn_Right = "Right";
	const char *btn_A = "A";
	const char *btn_B = "B";
	const char *btn_X = "X";
	const char *btn_Y = "Y";
	const char *btn_L = "L";
	const char *btn_R = "R";
	const char *btn_ZL = "ZL";
	const char *btn_ZR = "ZR";
	const char *btn_LS = "L-Stick";
	const char *btn_RS = "R-Stick";
	const char *btn_Start = "+";
	const char *btn_Select = "-";
#else
	const char *btn_Up = "Up";
	const char *btn_Down = "Down";
	const char *btn_Left = "Left";
	const char *btn_Right = "Right";
	const char *btn_A = "A";
	const char *btn_B = "B";
	const char *btn_X = "X";
	const char *btn_Y = "Y";
	const char *btn_L = "L";
	const char *btn_R = "R";
	const char *btn_ZL = "ZL";
	const char *btn_ZR = "ZR";
	const char *btn_LS = "L-Stick";
	const char *btn_RS = "R-Stick";
	const char *btn_Start = "Start";
	const char *btn_Select = "Select";
#endif

///////////////////
// MAIN FUNCTION //
///////////////////

void handlePlayerInput() {
	heldKeys = 0;
	handleAllCurrentInputs();
	updateChangedKeys();
	handleHoldTimer_prepare();
	handleHoldTimer_execute();
}
