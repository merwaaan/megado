#pragma once

#include <stdint.h>

typedef struct Metric {
    float* values;
    float avg;
    unsigned int idx;
    unsigned int length;
} Metric;

Metric* metric_make();
void metric_free(Metric*);
void metric_push(Metric*, float);
void metric_avg(Metric*);
void metric_plot(Metric*, char*);
