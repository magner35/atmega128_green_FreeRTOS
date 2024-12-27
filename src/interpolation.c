
#include "interpolation.h"

float interpolationStep(float xValues[], float yValues[], int numValues, float pointX, float threshold)
{
    // extremes
    if (pointX <= xValues[0])
        return yValues[0];
    if (pointX >= xValues[numValues - 1])
        return yValues[numValues - 1];

    int i = 0;
    while (pointX >= xValues[i + 1])
        i++;
    if (pointX == xValues[i + 1])
        return yValues[i + 1]; // exact match

    float t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]); // relative point in the interval
    return t < threshold ? yValues[i] : yValues[i + 1];
}

float interpolationLinear(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
    if (trim)
    {
        if (pointX <= xValues[0])
            return yValues[0];
        if (pointX >= xValues[numValues - 1])
            return yValues[numValues - 1];
    }

    int i = 0;
    float rst = 0;
    if (pointX <= xValues[0])
    {
        i = 0;
        float t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
        rst = yValues[i] * (1 - t) + yValues[i + 1] * t;
    }
    else if (pointX >= xValues[numValues - 1])
    {
        float t = (pointX - xValues[numValues - 2]) / (xValues[numValues - 1] - xValues[numValues - 2]);
        rst = yValues[numValues - 2] * (1 - t) + yValues[numValues - 1] * t;
    }
    else
    {
        while (pointX >= xValues[i + 1])
            i++;
        float t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
        rst = yValues[i] * (1 - t) + yValues[i + 1] * t;
    }

    return rst;
}

float interpolationSmoothStep(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
    if (trim)
    {
        if (pointX <= xValues[0])
            return yValues[0];
        if (pointX >= xValues[numValues - 1])
            return yValues[numValues - 1];
    }

    int i = 0;
    if (pointX <= xValues[0])
        i = 0;
    else if (pointX >= xValues[numValues - 1])
        i = numValues - 1;
    else
        while (pointX >= xValues[i + 1])
            i++;
    if (pointX == xValues[i + 1])
        return yValues[i + 1];

    float t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
    t = t * t * (3 - 2 * t);
    return yValues[i] * (1 - t) + yValues[i + 1] * t;
}

float catmullSlope(float x[], float y[], int n, int i)
{
    if (x[i + 1] == x[i - 1])
        return 0;
    return (y[i + 1] - y[i - 1]) / (x[i + 1] - x[i - 1]);
}

float interpolationCatmullSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
    if (trim)
    {
        if (pointX <= xValues[0])
            return yValues[0];
        if (pointX >= xValues[numValues - 1])
            return yValues[numValues - 1];
    }

    int i = 0;
    if (pointX <= xValues[0])
        i = 0;
    else if (pointX >= xValues[numValues - 1])
        i = numValues - 1;
    else
        while (pointX >= xValues[i + 1])
            i++;
    if (pointX == xValues[i + 1])
        return yValues[i + 1];

    float t = (pointX - xValues[i]) / (xValues[i + 1] - xValues[i]);
    float t_2 = t * t;
    float t_3 = t_2 * t;

    float h00 = 2 * t_3 - 3 * t_2 + 1;
    float h10 = t_3 - 2 * t_2 + t;
    float h01 = 3 * t_2 - 2 * t_3;
    float h11 = t_3 - t_2;

    float x0 = xValues[i];
    float x1 = xValues[i + 1];
    float y0 = yValues[i];
    float y1 = yValues[i + 1];

    float m0;
    float m1;
    if (i == 0)
    {
        m0 = (yValues[1] - yValues[0]) / (xValues[1] - xValues[0]);
        m1 = (yValues[2] - yValues[0]) / (xValues[2] - xValues[0]);
    }
    else if (i == numValues - 2)
    {
        m0 = (yValues[numValues - 1] - yValues[numValues - 3]) / (xValues[numValues - 1] - xValues[numValues - 3]);
        m1 = (yValues[numValues - 1] - yValues[numValues - 2]) / (xValues[numValues - 1] - xValues[numValues - 2]);
    }
    else
    {
        m0 = catmullSlope(xValues, yValues, numValues, i);
        m1 = catmullSlope(xValues, yValues, numValues, i + 1);
    }

    float rst = h00 * y0 + h01 * y1 + h10 * (x1 - x0) * m0 + h11 * (x1 - x0) * m1;
    return rst;
}

float getFirstDerivate(float x[], float y[], int n, int i)
{
    float fd1_x;

    if (i == 0)
    {
        fd1_x = 3.0f / 2.0f * (y[1] - y[0]) / (x[1] - x[0]);
        fd1_x -= getFirstDerivate(x, y, n, 1) / 2.0f;
    }
    else if (i == n)
    {
        fd1_x = 3.0f / 2.0f * (y[n] - y[n - 1]) / (x[n] - x[n - 1]);
        fd1_x -= getFirstDerivate(x, y, n, n - 1) / 2.0f;
    }
    else
    {
        if ((x[i + 1] - x[i]) / (y[i + 1] - y[i]) * (x[i] - x[i - 1]) / (y[i] - y[i - 1]) < 0)
        {
            fd1_x = 0;
        }
        else
        {
            fd1_x = 2.0f / ((x[i + 1] - x[i]) / (y[i + 1] - y[i]) + (x[i] - x[i - 1]) / (y[i] - y[i - 1]));
        }
    }
    return fd1_x;
}

float getLeftSecondDerivate(float x[], float y[], int n, int i)
{
    float fdi_x = getFirstDerivate(x, y, n, i);
    float fdi_xl1 = getFirstDerivate(x, y, n, i - 1);

    float fd2l_x = -2.0f * (fdi_x + 2.0f * fdi_xl1) / (x[i] - x[i - 1]);
    fd2l_x += 6.0f * (y[i] - y[i - 1]) / (x[i] - x[i - 1]) / (x[i] - x[i - 1]);

    return fd2l_x;
}

float getRightSecondDerivate(float x[], float y[], int numValues, int i)
{
    float fdi_x = getFirstDerivate(x, y, numValues, i);
    float fdi_xl1 = getFirstDerivate(x, y, numValues, i - 1);

    float fd2r_x = 2.0f * (2.0f * fdi_x + fdi_xl1) / (x[i] - x[i - 1]);
    fd2r_x -= 6.0f * (y[i] - y[i - 1]) / (x[i] - x[i - 1]) / (x[i] - x[i - 1]);

    return fd2r_x;
}

float interpolationConstrainedSpline(float xValues[], float yValues[], int numValues, float pointX, bool trim)
{
    if (trim)
    {
        if (pointX <= xValues[0])
            return yValues[0];
        if (pointX >= xValues[numValues - 1])
            return yValues[numValues - 1];
    }

    // auto i = 0;
    // while (pointX >= xValues[i + 1]) i++;
    // if (pointX == xValues[i + 1]) return yValues[i + 1];

    int i = 0;
    if (pointX <= xValues[0])
        i = 0;
    else if (pointX >= xValues[numValues - 1])
        i = numValues - 1;
    else
        while (pointX >= xValues[i + 1])
            i++;
    if (pointX == xValues[i + 1])
        return yValues[i + 1];

    float x0 = xValues[i + 1];
    float x1 = xValues[i];
    float y0 = yValues[i + 1];
    float y1 = yValues[i];

    float fd2i_xl1 = getLeftSecondDerivate(xValues, yValues, numValues - 1, i + 1);
    float fd2i_x = getRightSecondDerivate(xValues, yValues, numValues - 1, i + 1);

    float d = (fd2i_x - fd2i_xl1) / (6.0f * (x0 - x1));
    float c = (x0 * fd2i_xl1 - x1 * fd2i_x) / 2.0f / (x0 - x1);
    float b = (y0 - y1 - c * (x0 * x0 - x1 * x1) - d * (x0 * x0 * x0 - x1 * x1 * x1)) / (x0 - x1);
    float a = y1 - b * x1 - c * x1 * x1 - d * x1 * x1 * x1;

    float rst = a + pointX * (b + pointX * (c + pointX * d));
    return rst;
}
