#include <algorithm>
#include <stdexcept>
#include "actor/abilities/coyote_time.hpp"

CoyoteTime::CoyoteTime(float duration) : coyoteDuration(duration)
{
    if (duration <= 0)
        throw std::runtime_error("duration must be greater than 0");
}

void CoyoteTime::update(bool eligibleForCoyoteTime, float dt)
{
    if (eligibleForCoyoteTime)
        coyoteTimer = coyoteDuration;
    else
        coyoteTimer = std::max(0.0f, coyoteTimer - dt);
}

bool CoyoteTime::isCoyoteAvailable() const
{
    return coyoteTimer > 0.0f;
}

void CoyoteTime::consume()
{
    coyoteTimer = 0.0f;
}
