#pragma once

#include <stdbool.h>

extern bool ap_isEnabled;
extern const int AP_SCORE_THRESHOLD;
extern int ap_pointsUntilHint;
extern char ap_room[5];

extern void ap_handleHintCheck();
extern void ap_handleInputCheck();
extern void ap_handleLoginScreen();
extern void ap_enableArchipelago();
extern void ap_handleHintScreen();