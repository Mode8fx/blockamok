#pragma once

#include <SDL.h>
#include <stdbool.h>

extern Uint32 deltaTime;

extern SDL_GameController* controllers[4];

/* General Input */
typedef struct {
  Sint16 x;
  Sint16 x_last;
  Sint16 y;
  Sint16 y_last;
  const Sint16 deadZone;
  const Sint16 fullZone;
  const Sint16 threshold;
} Stick;
extern Stick leftStick;
extern Stick rightStick;

extern Uint32 pressedKeys;
extern Uint32 releasedKeys;
extern Uint32 heldKeys;
extern Uint32 heldKeys_last;

extern bool quit;

extern const char *btn_Up;
extern const char *btn_Down;
extern const char *btn_Left;
extern const char *btn_Right;
extern const char *btn_A;
extern const char *btn_B;
extern const char *btn_X;
extern const char *btn_Y;
extern const char *btn_L;
extern const char *btn_R;
extern const char *btn_ZL;
extern const char *btn_ZR;
extern const char *btn_LS;
extern const char *btn_RS;
extern const char *btn_Start;
extern const char *btn_Select;

#define INPUT_UP     (1 << 0)
#define INPUT_DOWN   (1 << 1)
#define INPUT_LEFT   (1 << 2)
#define INPUT_RIGHT  (1 << 3)
#define INPUT_A      (1 << 4)
#define INPUT_B      (1 << 5)
#define INPUT_X      (1 << 6)
#define INPUT_Y      (1 << 7)
#define INPUT_L      (1 << 8)
#define INPUT_R      (1 << 9)
#define INPUT_ZL     (1 << 10)
#define INPUT_ZR     (1 << 11)
#define INPUT_LS     (1 << 12)
#define INPUT_RS     (1 << 13)
#define INPUT_START  (1 << 14)
#define INPUT_SELECT (1 << 15)

#define STICK_DEADZONE 3277
#define STICK_FULLZONE 31129
#define STICK_THRESHOLD STICK_DEADZONE
#define TRIGGER_DEADZONE 26215

extern inline bool stickIsUp(Stick stick);
extern inline bool stickIsDown(Stick stick);
extern inline bool stickIsLeft(Stick stick);
extern inline bool stickIsRight(Stick stick);
extern bool keyPressed(Uint32);
extern bool keyHeld(Uint32);
//extern bool keyReleased(Uint32);
extern bool dirPressedUp();
extern bool dirPressedDown();
extern bool dirPressedLeft();
extern bool dirPressedRight();
extern bool dirHeld_Up();
extern bool dirHeld_Down();
extern bool dirHeld_Left();
extern bool dirHeld_Right();
extern void updateLastKeys();
extern void controllerInit();
extern void handlePlayerInput();
