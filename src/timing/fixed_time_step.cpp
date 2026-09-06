#include <algorithm>
#include <functional>
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr float Slack = 1e-6f;
}

FixedTimeStep::FixedTimeStep(float maxStep) : maxStep(maxStep)
{
}

void FixedTimeStep::run(float deltaTime, const std::function<void(float)> &stepFunc)
{
    carried += deltaTime;

    while (carried + Slack >= maxStep)
    {
        stepFunc(maxStep);
        carried = std::max(carried - maxStep, 0.0f);
    }
}

float FixedTimeStep::getMaxStep() const
{
    return maxStep;
}
