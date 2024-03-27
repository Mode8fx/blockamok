#include "./input.h"

#if defined(PSP)
SDL_Joystick *controller = NULL;
#else
SDL_GameController *controller = NULL;
#endif

/* General Input */
SDL_Event event;
Sint16 controllerAxis_leftStickX;
Sint16 controllerAxis_leftStickX_last;
Sint16 controllerAxis_leftStickY;
Sint16 controllerAxis_leftStickY_last;
Uint32 keyInputs;
Uint8  dirInputs;
Uint32 heldDirs;
#if defined(WII)
Uint32 wii_keysDown;
Uint32 wii_keysUp;
#elif defined(GAMECUBE)
Uint32 gc_keysDown;
Uint32 gc_keysUp;
#endif

double timer_buttonHold;
double timer_buttonHold_repeater;

bool quit = false;

bool buttonPressed(Uint32 key) {
	return (keyInputs & key);
}

bool dirHeld(Uint32 button) {
	return (heldDirs & button);
}

#if defined(PSP)
inline static void handleAnalogInput_PSP() {
	if (event.jaxis.which == 0) {
		if (event.jaxis.axis == 0) {
			controllerAxis_leftStickX = event.jaxis.value;
			if ((controllerAxis_leftStickX > -STICK_DEADZONE) && (controllerAxis_leftStickX < STICK_DEADZONE)) {
				controllerAxis_leftStickX = 0;
			}
		}
		if (event.jaxis.axis == 1) {
			controllerAxis_leftStickY = event.jaxis.value;
			if ((controllerAxis_leftStickY > -STICK_DEADZONE) && (controllerAxis_leftStickY < STICK_DEADZONE)) {
				controllerAxis_leftStickY = 0;
			}
		}
	}
}

inline static void handleButtonDown_PSP() {
	if (event.jbutton.which == 0) {
		if (event.jbutton.button == 8) { // Up
			dirInputs |= UP_PRESSED;
			return;
		}
		if (event.jbutton.button == 6) { // Down
			dirInputs |= DOWN_PRESSED;
			return;
		}
		if (event.jbutton.button == 7) { // Left
			dirInputs |= LEFT_PRESSED;
			return;
		}
		if (event.jbutton.button == 9) { // Right
			dirInputs |= RIGHT_PRESSED;
			return;
		}
		if (event.jbutton.button == 4) { // L
			keyInputs |= INPUT_L;
			return;
		}
		if (event.jbutton.button == 5) { // R
			keyInputs |= INPUT_R;
			return;
		}
		if (event.jbutton.button == 11) { // Start
			keyInputs |= INPUT_START;
			return;
		}
		if (event.jbutton.button == 10) { // Select
			keyInputs |= INPUT_SELECT;
			return;
		}
		//if (event.jbutton.button == 1) { // O
		//	return;
		//}
		if (event.jbutton.button == 2) { // X
			keyInputs |= INPUT_A;
			return;
		}
		//if (event.jbutton.button == 0) { // Triangle
		//	return;
		//}
		//if (event.cbutton.button == 3) { // Square
		//	return;
		//}
	}
}

inline static void handleButtonUp_PSP() {
	if (event.jbutton.button == 8) { // Up
		dirInputs |= UP_DEPRESSED;
		return;
	}
	if (event.jbutton.button == 6) { // Down
		dirInputs |= DOWN_DEPRESSED;
		return;
	}
	if (event.jbutton.button == 7) { // Left
		dirInputs |= LEFT_DEPRESSED;
		return;
	}
	if (event.jbutton.button == 9) { // Right
		dirInputs |= RIGHT_DEPRESSED;
		return;
	}
}

#elif defined(GAMECUBE)
inline static void gc_mapDir(Uint32 gcInput, Uint32 output) {
	if (gc_keysDown & gcInput) {
		dirInputs |= output;
	}
	else if (gc_keysUp & gcInput) {
		dirInputs |= output << 1;
	}
}

inline static void gc_mapButton(Uint32 gcInput, Uint32 output) {
	if (gc_keysDown & gcInput) {
		keyInputs |= output;
	}
	else if (gc_keysUp & gcInput) {
		keyInputs &= ~output;
	}
}

inline static void handleGCButtons() {
	gc_mapDir(PAD_BUTTON_UP, UP_PRESSED);
	gc_mapDir(PAD_BUTTON_DOWN, DOWN_PRESSED);
	gc_mapDir(PAD_BUTTON_LEFT, LEFT_PRESSED);
	gc_mapDir(PAD_BUTTON_RIGHT, RIGHT_PRESSED);
	gc_mapButton(PAD_BUTTON_A, INPUT_A);
	gc_mapButton(PAD_TRIGGER_L, INPUT_L);
	gc_mapButton(PAD_TRIGGER_R, INPUT_R);
	gc_mapButton(PAD_BUTTON_START, INPUT_START);
	gc_mapButton(PAD_BUTTON_Z, INPUT_SELECT);
	controllerAxis_leftStickX = PAD_StickX(0) * 256;
	controllerAxis_leftStickY = PAD_StickY(0) * -256;
	if ((controllerAxis_leftStickX > -STICK_DEADZONE) && (controllerAxis_leftStickX < STICK_DEADZONE)) {
		controllerAxis_leftStickX = 0;
	}
	if ((controllerAxis_leftStickY > -STICK_DEADZONE) && (controllerAxis_leftStickY < STICK_DEADZONE)) {
		controllerAxis_leftStickY = 0;
	}
}

#elif defined(WII)
inline static void wii_mapWiimoteDir(Uint32 wiimoteInput, Uint32 output) {
	if (wii_keysDown & wiimoteInput) {
		dirInputs |= output;
	}
	else if (wii_keysUp & wiimoteInput) {
		dirInputs |= output << 1;
	}
}

inline static void wii_mapWiimoteButton(Uint32 wiimoteInput, Uint32 output) {
	if (wii_keysDown & wiimoteInput) {
		keyInputs |= output;
	}
	else if (wii_keysUp & wiimoteInput) {
		keyInputs &= ~output;
	}
}

inline static void wii_mapWiiCCDir(Uint32 ccInput, Uint32 output) {
	if (wii_keysDown & ccInput) {
		dirInputs |= output;
	}
	else if (wii_keysUp & ccInput) {
		dirInputs |= output << 1;
	}
}

inline static void wii_mapWiiCCButton(Uint32 ccInput, Uint32 output) {
	if (wii_keysDown & ccInput) {
		keyInputs |= output;
	}
	else if (wii_keysUp & ccInput) {
		keyInputs &= ~output;
	}
}

inline static void wii_mapGCDir(Uint32 gcInput, Uint32 output) {
	if (wii_keysDown & gcInput) {
		dirInputs |= output;
	}
	else if (wii_keysUp & gcInput) {
		dirInputs |= output << 1;
	}
}

inline static void wii_mapGCButton(Uint32 gcInput, Uint32 output) {
	if (wii_keysDown & gcInput) {
		keyInputs |= output;
	}
	else if (wii_keysUp & gcInput) {
		keyInputs &= ~output;
	}
}

inline static void handleWiimoteButtons() {
	wii_mapWiimoteDir(WPAD_BUTTON_UP, LEFT_PRESSED);
	wii_mapWiimoteDir(WPAD_BUTTON_DOWN, RIGHT_PRESSED);
	wii_mapWiimoteDir(WPAD_BUTTON_LEFT, DOWN_PRESSED);
	wii_mapWiimoteDir(WPAD_BUTTON_RIGHT, UP_PRESSED);
	wii_mapWiimoteButton(WPAD_BUTTON_2, INPUT_A);
	wii_mapWiimoteButton(WPAD_BUTTON_B, INPUT_L);
	wii_mapWiimoteButton(WPAD_BUTTON_A, INPUT_R);
	wii_mapWiimoteButton(WPAD_BUTTON_PLUS, INPUT_START);
	wii_mapWiimoteButton(WPAD_BUTTON_HOME, INPUT_SELECT);
}

inline static void handleWiiCCButtons() {
	wii_mapWiiCCDir(WPAD_CLASSIC_BUTTON_LEFT, LEFT_PRESSED);
	wii_mapWiiCCDir(WPAD_CLASSIC_BUTTON_RIGHT, RIGHT_PRESSED);
	wii_mapWiiCCDir(WPAD_CLASSIC_BUTTON_DOWN, DOWN_PRESSED);
	wii_mapWiiCCDir(WPAD_CLASSIC_BUTTON_UP, UP_PRESSED);
	wii_mapWiiCCButton(WPAD_CLASSIC_BUTTON_A, INPUT_A);
	wii_mapWiiCCButton(WPAD_CLASSIC_BUTTON_FULL_L, INPUT_L);
	wii_mapWiiCCButton(WPAD_CLASSIC_BUTTON_FULL_R, INPUT_R);
	wii_mapWiiCCButton(WPAD_CLASSIC_BUTTON_PLUS, INPUT_START);
	wii_mapWiiCCButton(WPAD_CLASSIC_BUTTON_HOME, INPUT_SELECT);
}

inline static void handleWiiGCButtons() {
	wii_mapGCDir(PAD_BUTTON_UP, UP_PRESSED);
	wii_mapGCDir(PAD_BUTTON_DOWN, DOWN_PRESSED);
	wii_mapGCDir(PAD_BUTTON_LEFT, LEFT_PRESSED);
	wii_mapGCDir(PAD_BUTTON_RIGHT, RIGHT_PRESSED);
	wii_mapGCButton(PAD_BUTTON_A, INPUT_A);
	wii_mapGCButton(PAD_TRIGGER_L, INPUT_L);
	wii_mapGCButton(PAD_TRIGGER_R, INPUT_R);
	wii_mapGCButton(PAD_BUTTON_START, INPUT_START);
	wii_mapGCButton(PAD_TRIGGER_Z, INPUT_SELECT);
}

#else
inline static void handleAnalogInput_SDL2() {
	controllerAxis_leftStickX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
	controllerAxis_leftStickY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
	if ((controllerAxis_leftStickX > -STICK_DEADZONE) && (controllerAxis_leftStickX < STICK_DEADZONE)) {
		controllerAxis_leftStickX = 0;
	}
	if ((controllerAxis_leftStickY > -STICK_DEADZONE) && (controllerAxis_leftStickY < STICK_DEADZONE)) {
		controllerAxis_leftStickY = 0;
	}
}

inline static void handleButtonDown_SDL2() {
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
		dirInputs |= UP_PRESSED;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
		dirInputs |= DOWN_PRESSED;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
		dirInputs |= LEFT_PRESSED;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
		dirInputs |= RIGHT_PRESSED;
		return;
	}
//#if defined(SWITCH)
//	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
//#else
//	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
//#endif
//		keyInputs |= INPUT_CONFIRM;
//		return;
//	}
#if defined(SWITCH)
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_B) {
#else
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A) {
#endif
		keyInputs |= INPUT_A;
		return;
	}
	//if (event.cbutton.button == SDL_CONTROLLER_BUTTON_X) {
	//	return;
	//}
	//if (event.cbutton.button == SDL_CONTROLLER_BUTTON_Y) {
	//	return;
	//}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER) {
		keyInputs |= INPUT_L;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) {
		keyInputs |= INPUT_R;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
		keyInputs |= INPUT_START;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
		keyInputs |= INPUT_SELECT;
		return;
	}
}

inline static void handleButtonUp_SDL2() {
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
		dirInputs |= UP_DEPRESSED;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
		dirInputs |= DOWN_DEPRESSED;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
		dirInputs |= LEFT_DEPRESSED;
		return;
	}
	if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
		dirInputs |= RIGHT_DEPRESSED;
		return;
	}
}
#endif

inline static void handleKeyboardKeys() {
	if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w) {
		keyInputs |= INPUT_UP;
		return;
	}
	if (event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s) {
		keyInputs |= INPUT_DOWN;
		return;
	}
	if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a) {
		keyInputs |= INPUT_LEFT;
		return;
	}
	if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d) {
		keyInputs |= INPUT_RIGHT;
		return;
	}
	if (event.key.keysym.sym == SDLK_LSHIFT) {
		keyInputs |= INPUT_A;
		return;
	}
	if (event.key.keysym.sym == SDLK_MINUS) {
		keyInputs |= INPUT_L;
		return;
	}
	if (event.key.keysym.sym == SDLK_EQUALS) {
		keyInputs |= INPUT_R;
		return;
	}
	if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_RETURN2 || event.key.keysym.sym == SDLK_KP_ENTER) {
		keyInputs |= INPUT_START;
		return;
	}
	if (event.key.keysym.sym == SDLK_q) {
		keyInputs |= INPUT_SELECT;
		return;
	}
#if defined(ANDROID)
	if (event.key.keysym.sym == SDLK_AC_BACK) {
		keyInputs |= INPUT_START;
		return;
	}
#endif
}

//inline static void handleKeyboardKeysHeld() {
//	const Uint8* keyState = SDL_GetKeyboardState(NULL);
//	if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) {
//		dirInputs |= UP_PRESSED;
//	}
//	if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) {
//		dirInputs |= DOWN_PRESSED;
//	}
//	if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT]) {
//		dirInputs |= LEFT_PRESSED;
//	}
//	if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT]) {
//		dirInputs |= RIGHT_PRESSED;
//	}
//	if (keyState[SDL_SCANCODE_LSHIFT]) {
//		keyInputs |= INPUT_A;
//	}
//}

inline static void dirHandler(Uint8 pressedVal, Uint8 depressedVal, Uint8 inputVal) {
	if (dirInputs & pressedVal) {
		keyInputs |= inputVal;
		heldDirs |= inputVal;
	}
	else if (dirInputs & depressedVal) {
		heldDirs &= ~inputVal;
	}
}

inline static void handleInputPressDepress() {
	if ((controllerAxis_leftStickX < 0) && !(controllerAxis_leftStickX_last < 0)) {
		dirInputs |= LEFT_PRESSED;
	}
	else if (!(controllerAxis_leftStickX < 0) && (controllerAxis_leftStickX_last < 0)) { // a little redundant, but easier to read
		dirInputs |= LEFT_DEPRESSED;
	}
	if ((controllerAxis_leftStickX > 0) && !(controllerAxis_leftStickX_last > 0)) {
		dirInputs |= RIGHT_PRESSED;
	}
	else if (!(controllerAxis_leftStickX > 0) && (controllerAxis_leftStickX_last > 0)) {
		dirInputs |= RIGHT_DEPRESSED;
	}
	if ((controllerAxis_leftStickY < 0) && !(controllerAxis_leftStickY_last < 0)) {
		dirInputs |= UP_PRESSED;
	}
	else if (!(controllerAxis_leftStickY < 0) && (controllerAxis_leftStickY_last < 0)) {
		dirInputs |= UP_DEPRESSED;
	}
	if ((controllerAxis_leftStickY > 0) && !(controllerAxis_leftStickY_last > 0)) {
		dirInputs |= DOWN_PRESSED;
	}
	else if (!(controllerAxis_leftStickY > 0) && (controllerAxis_leftStickY_last > 0)) {
		dirInputs |= DOWN_DEPRESSED;
	}
	dirHandler(UP_PRESSED, UP_DEPRESSED, INPUT_UP);
	dirHandler(DOWN_PRESSED, DOWN_DEPRESSED, INPUT_DOWN);
	dirHandler(LEFT_PRESSED, LEFT_DEPRESSED, INPUT_LEFT);
	dirHandler(RIGHT_PRESSED, RIGHT_DEPRESSED, INPUT_RIGHT);
	if (timer_buttonHold > 0.5) {
		timer_buttonHold_repeater += deltaTime;
		if (timer_buttonHold_repeater >= 0.033) {
			if (dirHeld(INPUT_UP)) {
				keyInputs |= INPUT_UP;
			}
			if (dirHeld(INPUT_DOWN)) {
				keyInputs |= INPUT_DOWN;
			}
			if (dirHeld(INPUT_LEFT)) {
				keyInputs |= INPUT_LEFT;
			}
			if (dirHeld(INPUT_RIGHT)) {
				keyInputs |= INPUT_RIGHT;
			}
			timer_buttonHold_repeater -= 0.033;
		}
	}
}

void handlePlayerInput() {
	if (heldDirs > 0) {
		timer_buttonHold += deltaTime;
	}
	else {
		timer_buttonHold = 0;
		timer_buttonHold_repeater = 0;
	}

	keyInputs = 0;
	dirInputs = 0;
#if defined(PSP)
	/* Handle Key Presses (PSP) */
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			/* Handle Analog Input (PSP) */
			case SDL_JOYAXISMOTION:
				handleAnalogInput_PSP();
				break;
			/* Handle Button Input (PSP) */
			case SDL_JOYBUTTONDOWN:
				handleButtonDown_PSP();
				break;
			case SDL_JOYBUTTONUP:
				handleButtonUp_PSP();
				break;
			default:
				break;
		}
	}
#elif defined(GAMECUBE)
	PAD_ScanPads();
	gc_keysDown = PAD_ButtonsDown(0);
	gc_keysUp = PAD_ButtonsUp(0);
	handleGCButtons();
#elif defined(WII)
	WPAD_ScanPads();
	wii_keysDown = WPAD_ButtonsDown(0);
	wii_keysUp = WPAD_ButtonsUp(0);
	handleWiimoteButtons();
	handleWiiCCButtons();

	PAD_ScanPads();
	wii_keysDown = PAD_ButtonsDown(0);
	wii_keysUp = PAD_ButtonsUp(0);
	handleWiiGCButtons();

	controllerAxis_leftStickX = PAD_StickX(0) * 256;
	controllerAxis_leftStickY = PAD_StickY(0) * -256;
	if ((controllerAxis_leftStickX > -STICK_DEADZONE) && (controllerAxis_leftStickX < STICK_DEADZONE)) {
		controllerAxis_leftStickX = 0;
	}
	if ((controllerAxis_leftStickY > -STICK_DEADZONE) && (controllerAxis_leftStickY < STICK_DEADZONE)) {
		controllerAxis_leftStickY = 0;
	}
#else
	/* Handle Analog Input (SDL2) */
	handleAnalogInput_SDL2();
	/* Handle Held Keyboard Keys (PC) */
	//handleKeyboardKeysHeld();
	/* Handle Key Presses and Mouse Input (SDL2) */
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			/* Handle Keyboard Input (PC) */
			case SDL_KEYDOWN:
				handleKeyboardKeys();
				break;
			/* Handle Button Input (SDL2) */
			case SDL_CONTROLLERBUTTONDOWN:
				handleButtonDown_SDL2();
				break;
			case SDL_CONTROLLERBUTTONUP:
				handleButtonUp_SDL2();
				break;
			default:
				break;
		}
	}
#endif
	/* Handle Press/Depress/Hold */
	handleInputPressDepress();
}

void controllerInit() {
#if defined(PSP)
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	controller = SDL_JoystickOpen(0);
#else
	for (Sint8 controllerNum = 0; controllerNum < SDL_NumJoysticks(); controllerNum++) {
		if (SDL_IsGameController(controllerNum)) {
			controller = SDL_GameControllerOpen(controllerNum);
			break;
		}
	}
#endif
}