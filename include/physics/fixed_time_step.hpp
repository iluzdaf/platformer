#pragma once

#include <functional>

class FixedTimeStep
{
public:
    explicit FixedTimeStep(float maxStep = 0.01f);

    void run(float deltaTime, const std::function<void(float)> &stepFunc) const;

private:
    float maxStep;
};