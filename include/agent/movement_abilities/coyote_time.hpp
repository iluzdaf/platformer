#pragma once

#include <algorithm>
#include <stdexcept>

struct CoyoteTime
{
    CoyoteTime(float duration = 0.1f) : coyoteDuration(duration)
    {
        if (duration <= 0)
            throw std::runtime_error("duration must be greater than 0");
    }

    void update(bool eligibleForCoyoteTime, float dt)
    {
        if (eligibleForCoyoteTime)
            coyoteTimer = coyoteDuration;
        else
            coyoteTimer = std::max(0.0f, coyoteTimer - dt);
    }

    bool isCoyoteAvailable() const { return coyoteTimer > 0.0f; }
    void consume() { coyoteTimer = 0.0f; }
    void setDuration(float duration) { coyoteDuration = duration; }

    float coyoteDuration = 0.1f;
    float coyoteTimer = 0.0f;
};