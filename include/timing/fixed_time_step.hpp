#pragma once

#include <functional>

inline constexpr float PhysicsStep = 0.01f;

class FixedTimeStep
{
public:
    explicit FixedTimeStep(float maxStep = PhysicsStep);
    void run(float deltaTime, const std::function<void(float)> &stepFunc);
    float getMaxStep() const;

private:
    float maxStep;
    float carried = 0.0f;
};
