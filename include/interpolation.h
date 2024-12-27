#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_

#include <stdbool.h>

float interpolationStep(float xValues[], float yValues[], int numValues, float pointX, float threshold);
float interpolationLinear(float xValues[], float yValues[], int numValues, float pointX, bool trim);
float interpolationSmoothStep(float xValues[], float yValues[], int numValues, float pointX, bool trim);
float interpolationCatmullSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim);
float interpolationConstrainedSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim);

#endif // !_INTERPOLATION_H_