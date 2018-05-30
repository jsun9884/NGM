#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

class Filter
{
public:
    Filter(float *buf, int size, int stride);

    float movingAverage(float in);
    float movingAverageDeviation();

    static float average(float *DataInOut, const float *filterIn, float in, int16_t filter_size);
    static float smoothing(float prev_Out, float In , float Alpha_coef);
    static float clipper(float act_in, float prev_in);

private:
    float _sum;
    float *_buffer;
    int _pos;
    int _size;
    int _stride;
};

#endif // FILTER_H
