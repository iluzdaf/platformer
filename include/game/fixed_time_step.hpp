#pragma once

#include <functional>

class FixedTimeStep
{
public:
    explicit FixedTimeStep(float maxStep = 0.01f)
        : maxStep(maxStep) {}

    void run(float deltaTime, const std::function<void(float)> &stepFunc) const
    {
        while (deltaTime > 0.0f)
        {
            float dt = std::min(maxStep, deltaTime);
            stepFunc(dt);
            deltaTime -= dt;
        }
    }

private:
    float maxStep;
};