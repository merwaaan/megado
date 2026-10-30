#pragma once

#include <cstddef>
#include <vector>

// Ring buffer of recorded values, used to compute a moving average of a metric
class Metric
{
public:
    explicit Metric(size_t length);

    void push(float value);
    void compute_average();

    std::vector<float> values;
    size_t index = 0;

    float average = 0.0f;
};
