#pragma once

#include <stdint.h>

// Ring buffer of recorded values, used to compute a moving average of a metric
typedef struct Metric
{
    float* values;
    unsigned int values_length;

    unsigned int index;

    float average;
} Metric;

Metric* metric_make();
void metric_free(Metric*);
void metric_push(Metric*, float);
void metric_average(Metric*);
