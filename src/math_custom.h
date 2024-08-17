#include "./draw.h"

#define FOV_ANGLE 45
#define ADJUSTED_FOV (FOV_ANGLE * smallerAspectRatio)

extern inline float transform3Dto2D(float xy, float z);
extern inline float randF(float min, float max);