#include "./math_custom.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define M_PI 3.14159265358979323846

float transform3Dto2D(float xy, float z) {
  return xy / ((z)*tan(HALF_FOV_ANGLE_RADIANS));
}

float randF(float min, float max) {
  return min + (float)rand() / ((float)RAND_MAX / (max - min));
}