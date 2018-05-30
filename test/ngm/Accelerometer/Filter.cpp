#include <math.h>

#include "Filter.h"

#define CLIPPER_DELTA_MAX   0.5f

//Saturate X at +/-max
#define SATURATE(x, max) ( ((x) > (max)) ? (max) : ( ((x) < -(max)) ? (-(max)) : (x) ) )

Filter::Filter(float *buf, int size, int stride):
    _sum(0.0f), _buffer(buf), _pos(0), _size(size), _stride(stride)
{
    for (int i = 0; i < size; ++i) {
        buf[i * stride] = 0.0f;
    }
}

float Filter::average(float *DataInOut, const float *filterIn, float in, int16_t filter_size)
{
    int16_t i;
    float out = 0.0f;

    for (i = filter_size-1 ; i > 0 ; i--)
        DataInOut[i] = DataInOut[i-1];
    DataInOut[0] = in;

    for (i=0; i < filter_size; i++)
        out += DataInOut[i]*filterIn[i];

    return out;
}

float Filter::movingAverage(float in)
{
    int offset = _pos * _stride;

    _sum += in - _buffer[offset];
    _buffer[offset] = in;

    if (++_pos >= _size) {
        _pos = 0;
    }

    return _sum / _size;
}

float Filter::movingAverageDeviation()
{
    float sum = 0;
    float average = _sum / _size;

    for (int i = 0; i < _size; i++)
    {
        sum += fabs(average - _buffer[i * _stride]);
    }

    return sum / _size;
}

float Filter::smoothing(float prev_Out, float In, float Alpha_coef)
{
    return Alpha_coef * prev_Out + (1 - Alpha_coef) * In;
}

float Filter::clipper(float act_in, float prev_in)
{
    float delta;

    delta = SATURATE(act_in - prev_in, CLIPPER_DELTA_MAX);
    return (prev_in + delta);
}

