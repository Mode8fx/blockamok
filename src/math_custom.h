#include "./draw.h"

#define FOV_ANGLE 45
//#define ADJUSTED_FOV (FOV_ANGLE * WIDTH / HEIGHT)
#define ADJUSTED_FOV (FOV_ANGLE * HEIGHT / WIDTH)

float transform3Dto2D(float xy, float z);
float randF(float min, float max);