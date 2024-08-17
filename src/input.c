#include "./input.h"

#if defined(PSP)
SDL_Joystick *controller = NULL;
#else
SDL_GameController *controller = NULL;
#endif

/* General Input */
Sint16 controllerAxis_leftStickX;
Sint16 controllerAxis_leftStickX_last;
Sint16 controllerAxis_leftStickY;
Sint16 controllerAxis_leftStickY_last;
Sint16 controllerAxis_rightStickX;
Sint16 controllerAxis_rightStickX_last;
Sint16 controllerAxis_rightStickY;
Sint16 controllerAxis_rightStickY_last;
Uint32 pressedKeys;
Uint32 releasedKeys;
Uint32 heldKeys;
Uint32 heldKeys_last;

double timer_buttonHold;
double timer_buttonHold_repeater;
#define REPEATER_FIRST_DELAY 0.5
#define REPEATER_INTERVAL 0.033

bool quit = false;

inline bool keyPressed(Uint32 key) {
	return (pressedKeys & key);
}

inline bool keyHeld(Uint32 key) {
	return (heldKeys & key);
}

static inline void handleHoldTimer_prepare() {
	// If any direction is held, continue timer; otherwise, reset it
	if (heldKeys & 0x0F) {
		timer_buttonHold += deltaTime;
	}
	else {
		timer_buttonHold = 0;
		timer_buttonHold_repeater = 0;
	}
}

static inline void updateChangedKeys() {
	pressedKeys = heldKeys & ~heldKeys_last;
	releasedKeys = ~heldKeys & heldKeys_last;
}

static inline void handleHoldTimer_execute() {
	// If a direction is held, "press" it again
	if (timer_buttonHold > REPEATER_FIRST_DELAY) {
		timer_buttonHold_repeater += deltaTime;
		if (timer_buttonHold_repeater >= REPEATER_INTERVAL) {
			if (keyHeld(INPUT_UP)) {
				pressedKeys |= INPUT_UP;
			}
			if (keyHeld(INPUT_DOWN)) {
				pressedKeys |= INPUT_DOWN;
			}
			if (keyHeld(INPUT_LEFT)) {
				pressedKeys |= INPUT_LEFT;
			}
			if (keyHeld(INPUT_RIGHT)) {
				pressedKeys |= INPUT_RIGHT;
			}
			timer_buttonHold_repeater -= REPEATER_INTERVAL;
		}
	}
}

static inline void updateLastKeys() {
	heldKeys_last = heldKeys;
	controllerAxis_leftStickX_last = controllerAxis_leftStickX;
	controllerAxis_leftStickY_last = controllerAxis_leftStickY;
	controllerAxis_rightStickX_last = controllerAxis_rightStickX;
	controllerAxis_rightStickY_last = controllerAxis_rightStickY;
}

inline void handlePlayerInput() {
	heldKeys = 0;
	handleAllCurrentInputs();
	updateChangedKeys();
	handleHoldTimer_prepare();
	handleHoldTimer_execute();
	updateLastKeys();
	printf("%u\n", heldKeys);
}

void controllerInit() {
#if defined(PSP)
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	controller = SDL_JoystickOpen(0);
#else
	Sint8 numJoysticks = SDL_NumJoysticks();
	for (Sint8 controllerNum = 0; controllerNum < numJoysticks; controllerNum++) {
		if (SDL_IsGameController(controllerNum)) {
			controller = SDL_GameControllerOpen(controllerNum);
			break;
		}
	}
#endif
}

#if !(defined(PSP) || defined(GAMECUBE) || defined(WII))
static inline void mapInputToVar_SDL2(Uint16 varBtn, Sint32 inputBtn) {
	if (SDL_GameControllerGetButton(controller, inputBtn)) {
		printf("BUTTON PRESSED\n");
		heldKeys |= varBtn;
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

#if defined(WII) || defined(GAMECUBE) || defined(PSP)
static inline void mapInputToVar_GeneralConsole(Uint32 heldButtons, Uint16 varBtn, Sint32 inputBtn) {
	if (heldButtons & inputBtn) {
		heldKeys |= varBtn;
	}
}
#endif

static inline void applyStickDeadzones() {
	if (abs(controllerAxis_leftStickX) < STICK_DEADZONE) controllerAxis_leftStickX = 0;
	if (abs(controllerAxis_leftStickY) < STICK_DEADZONE) controllerAxis_leftStickY = 0;
	if (abs(controllerAxis_rightStickX) < STICK_DEADZONE) controllerAxis_rightStickX = 0;
	if (abs(controllerAxis_rightStickY) < STICK_DEADZONE) controllerAxis_rightStickY = 0;
}

static void mapStickToVar() {
	if (controllerAxis_leftStickX < 0) {
		heldKeys |= INPUT_LEFT;
	}
	if (controllerAxis_leftStickX > 0) {
		heldKeys |= INPUT_RIGHT;
	}
	if (controllerAxis_leftStickY < 0) {
		heldKeys |= INPUT_UP;
	}
	if (controllerAxis_leftStickY > 0) {
		heldKeys |= INPUT_DOWN;
	}
}


static void handleAllCurrentInputs() {
#if !(defined(PSP) || defined(GAMECUBE) || defined(WII))
	mapInputToVar_SDL2(INPUT_UP, SDL_CONTROLLER_BUTTON_DPAD_UP);
	mapInputToVar_SDL2(INPUT_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	mapInputToVar_SDL2(INPUT_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	mapInputToVar_SDL2(INPUT_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	mapInputToVar_SDL2(INPUT_A, SDL_CONTROLLER_BUTTON_A);
	mapInputToVar_SDL2(INPUT_B, SDL_CONTROLLER_BUTTON_B);
	mapInputToVar_SDL2(INPUT_X, SDL_CONTROLLER_BUTTON_X);
	mapInputToVar_SDL2(INPUT_Y, SDL_CONTROLLER_BUTTON_Y);
	mapInputToVar_SDL2(INPUT_SELECT, SDL_CONTROLLER_BUTTON_BACK);
	mapInputToVar_SDL2(INPUT_START, SDL_CONTROLLER_BUTTON_START);
	mapInputToVar_SDL2(INPUT_L, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	mapInputToVar_SDL2(INPUT_R, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

	int leftTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
	int rightTrigger = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
	if (leftTrigger > TRIGGER_DEADZONE) {
		heldKeys |= INPUT_ZL;
	}
	if (rightTrigger > TRIGGER_DEADZONE) {
		heldKeys |= INPUT_ZR;
	}

	controllerAxis_leftStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
	controllerAxis_leftStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
	controllerAxis_rightStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
	controllerAxis_rightStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
	applyStickDeadzones();
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
	mapInputToVar_Keyboard(state, INPUT_L, SDL_SCANCODE_MINUS);
	mapInputToVar_Keyboard(state, INPUT_R, SDL_SCANCODE_EQUALS);
	mapInputToVar_Keyboard(state, INPUT_START, SDL_SCANCODE_RETURN);
	mapInputToVar_Keyboard(state, INPUT_START, SDL_SCANCODE_RETURN2);
	mapInputToVar_Keyboard(state, INPUT_START, SDL_SCANCODE_KP_ENTER);
	mapInputToVar_Keyboard(state, INPUT_SELECT, SDL_SCANCODE_Q);
#if defined(ANDROID)
	mapInputToVar_Button_Keyboard(state, INPUT_START, SDLK_AC_BACK);
#endif
#endif

#if defined(WII) || defined(GAMECUBE)
	PAD_ScanPads();
	u32 heldButtonsGC = PAD_ButtonsHeld(0);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_UP, PAD_BUTTON_UP);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_DOWN, PAD_BUTTON_DOWN);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_LEFT, PAD_BUTTON_LEFT);
	mapInputToVar_GeneralConsole(heldButtonsGC, INPUT_RIGHT, PAD_BUTTON_RIGHT);
	mapInputToVar_GeneralConsole(INPUT_A, PAD_BUTTON_A);
	mapInputToVar_GeneralConsole(INPUT_B, PAD_BUTTON_B);
	mapInputToVar_GeneralConsole(INPUT_X, PAD_BUTTON_X);
	mapInputToVar_GeneralConsole(INPUT_Y, PAD_BUTTON_Y);
	mapInputToVar_GeneralConsole(INPUT_L, PAD_TRIGGER_L);
	mapInputToVar_GeneralConsole(INPUT_R, PAD_TRIGGER_R);
	mapInputToVar_GeneralConsole(INPUT_START, PAD_BUTTON_START);
	mapInputToVar_GeneralConsole(INPUT_SELECT, PAD_TRIGGER_Z);

	controllerAxis_leftStickX = PAD_StickX(0) * 256;
	controllerAxis_leftStickY = PAD_StickY(0) * -256;
	controllerAxis_rightStickX = PAD_SubStickX(0) * 256;
	controllerAxis_rightStickY = PAD_SubStickY(0) * -256;
	applyStickDeadzones();
#endif

#if defined(WII)
	WPAD_ScanPads();
	u32 heldButtonsWii = WPAD_ButtonsHeld(0);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_UP, WPAD_BUTTON_RIGHT);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_DOWN, WPAD_BUTTON_LEFT);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_LEFT, WPAD_BUTTON_UP);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_RIGHT, WPAD_BUTTON_DOWN);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_A, WPAD_BUTTON_2);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_B, WPAD_BUTTON_1);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_L, WPAD_BUTTON_B);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_R, WPAD_BUTTON_A);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_START, WPAD_BUTTON_PLUS);
	mapInputToVar_GeneralConsole(heldButtonsWii, INPUT_SELECT, WPAD_BUTTON_MINUS);

	mapInputToVar_GeneralConsole(INPUT_UP, WPAD_CLASSIC_BUTTON_UP);
	mapInputToVar_GeneralConsole(INPUT_DOWN, WPAD_CLASSIC_BUTTON_DOWN);
	mapInputToVar_GeneralConsole(INPUT_LEFT, WPAD_CLASSIC_BUTTON_LEFT);
	mapInputToVar_GeneralConsole(INPUT_RIGHT, WPAD_CLASSIC_BUTTON_RIGHT);
	mapInputToVar_GeneralConsole(INPUT_A, WPAD_CLASSIC_BUTTON_A);
	mapInputToVar_GeneralConsole(INPUT_B, WPAD_CLASSIC_BUTTON_B);
	mapInputToVar_GeneralConsole(INPUT_X, WPAD_CLASSIC_BUTTON_X);
	mapInputToVar_GeneralConsole(INPUT_Y, WPAD_CLASSIC_BUTTON_Y);
	mapInputToVar_GeneralConsole(INPUT_L, WPAD_CLASSIC_BUTTON_FULL_L);
	mapInputToVar_GeneralConsole(INPUT_R, WPAD_CLASSIC_BUTTON_FULL_R);
	mapInputToVar_GeneralConsole(INPUT_START, WPAD_CLASSIC_BUTTON_PLUS);
	mapInputToVar_GeneralConsole(INPUT_SELECT, WPAD_CLASSIC_BUTTON_MINUS);

	// Gamecube Controller input should supercede Classic Controller
	if (controllerAxis_leftStickX == 0) {
		controllerAxis_leftStickX = WPAD_Stick(WPAD_CHAN_0, 0, 0) * 256;
	}
	if (controllerAxis_leftStickY == 0) {
		controllerAxis_leftStickY = WPAD_Stick(WPAD_CHAN_0, 0, 1) * -256;
	}
	if (controllerAxis_rightStickX == 0) {
		controllerAxis_rightStickX = WPAD_Stick(WPAD_CHAN_0, 1, 0) * 256;
	}
	if (controllerAxis_rightStickY == 0) {
		controllerAxis_rightStickY = WPAD_Stick(WPAD_CHAN_0, 1, 1) * -256;
	}
	applyStickDeadzones();
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

	controllerAxis_leftStickX = (pad.Lx - 128) * 256;
	controllerAxis_leftStickY = (pad.Ly - 128) * 256;
	applyStickDeadzones();
#endif

	mapStickToVar();
}
