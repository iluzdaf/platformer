#include <algorithm>
#include <stdexcept>
#include "actor/abilities/grace_period.hpp"

GracePeriod::GracePeriod(float length) : length(length)
{
    if (length <= 0.0f)
        throw std::runtime_error("A grace period needs a length above 0");
}

void GracePeriod::start(float direction)
{
    left = length;
    startedDirection = direction;
}

void GracePeriod::update(float deltaTime)
{
    left = std::max(0.0f, left - deltaTime);
}

void GracePeriod::consume()
{
    left = 0.0f;
    startedDirection = 0.0f;
}

bool GracePeriod::running() const
{
    return left > 0.0f;
}

float GracePeriod::direction() const
{
    return running() ? startedDirection : 0.0f;
}
