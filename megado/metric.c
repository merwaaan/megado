#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui/cimgui.h>
#include <stdlib.h>

#include "metric.h"

Metric* metric_make(unsigned int length) {
    Metric* m = calloc(1, sizeof(Metric));
    m->values = calloc(length, sizeof(float));
    m->length = length;

    return m;
}

void metric_free(Metric* m) {
    free(m->values);
    m->values = NULL;
    free(m);
    m = NULL;
}

void metric_push(Metric* m, float value) {
    m->values[m->idx] = value;
    m->idx = (m->idx + 1) % m->length;
}

void metric_avg(Metric* m) {
    m->avg = 0;
    for (unsigned int i=0; i < m->length; ++i) {
        m->avg += m->values[i];
    }
    m->avg /= m->length;
}

void metric_plot(Metric* m, char* title) {
    struct ImVec2 size = { 0, 40 };
    igPlotHistogram(title, m->values, m->length, m->idx,
                    NULL, 0, m->avg * 2, size, sizeof(float));
}
