#pragma once

#include <SDL.h>
#include <stdbool.h>

extern double deltaTime;

#if defined(PSP)
extern SDL_Joystick *controller;
#else
extern SDL_GameController *controller;
#endif

/* General Input */
extern SDL_Event event;
extern Sint16 controllerAxis_leftStickX;
extern Sint16 controllerAxis_leftStickX_last;
extern Sint16 controllerAxis_leftStickY;
extern Sint16 controllerAxis_leftStickY_last;
extern Uint32 keyInputs;
extern Uint8  dirInputs;
extern Uint32 heldDirs;

extern bool quit;

#define INPUT_UP     1
#define INPUT_DOWN   2
#define INPUT_LEFT   4
#define INPUT_RIGHT  8
#define INPUT_A      16
#define INPUT_L      32
#define INPUT_R      64
#define INPUT_START  128
#define INPUT_SELECT 256

#define STICK_DEADZONE 13107

#define LEFT_PRESSED    1
#define LEFT_DEPRESSED  2
#define RIGHT_PRESSED   4
#define RIGHT_DEPRESSED 8
#define UP_PRESSED      16
#define UP_DEPRESSED    32
#define DOWN_PRESSED    64
#define DOWN_DEPRESSED  128

extern bool buttonPressed(Uint32);
extern bool dirHeld(Uint32);
extern void handlePlayerInput();
extern void controllerInit();