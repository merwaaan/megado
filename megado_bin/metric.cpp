#include <numeric>

#include "metric.h"

using namespace std;

Metric::Metric(size_t length)
    : values(length, 0.0f)
{
}

void Metric::push(float value)
{
    values[index] = value;
    index = (index + 1) % values.size();
}

void Metric::compute_average()
{
    average = accumulate(values.begin(), values.end(), 0.0f) / values.size();
}
