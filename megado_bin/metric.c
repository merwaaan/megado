#include <stdlib.h>

#include "metric.h"

Metric* metric_make(unsigned int length)
{
    Metric* m = calloc(1, sizeof(Metric));
    m->values = calloc(length, sizeof(float));
    m->values_length = length;
    m->index = 0;
    m->average = 0.0f;

    return m;
}

void metric_free(Metric* m)
{
    free(m->values);
    free(m);
}

void metric_push(Metric* m, float value)
{
    m->values[m->index] = value;
    m->index = (m->index + 1) % m->values_length;
}

void metric_average(Metric* m)
{
    m->average = 0;

    for (unsigned int i = 0; i < m->values_length; ++i)
    {
        m->average += m->values[i];
    }

    m->average /= m->values_length;
}
