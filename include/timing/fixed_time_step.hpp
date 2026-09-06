#pragma once

#include <functional>

class FixedTimeStep
{
public:
    explicit FixedTimeStep(float maxStep = 0.01f);
    void run(float deltaTime, const std::function<void(float)> &stepFunc);
    float getMaxStep() const;

private:
    float maxStep;
    float carried = 0.0f;
};
