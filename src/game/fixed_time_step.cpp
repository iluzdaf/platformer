#include "game/fixed_time_step.hpp"
#include <algorithm>

FixedTimeStep::FixedTimeStep(float maxStep)
    : maxStep(maxStep) {}

void FixedTimeStep::run(float deltaTime, const std::function<void(float)> &stepFunc) const
{
    while (deltaTime > 0.0f)
    {
        float dt = std::min(maxStep, deltaTime);
        stepFunc(dt);
        deltaTime -= dt;
    }
}