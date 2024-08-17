#include "./math_custom.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

inline float transform3Dto2D(float xy, float z) {
  return xy / ((z)*tan(HALF_FOV_ANGLE_RADIANS));
}

inline float randF(float min, float max) {
  return min + (float)rand() / ((float)RAND_MAX / (max - min));
}