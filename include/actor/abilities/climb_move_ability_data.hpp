#pragma once

struct ClimbMoveAbilityData
{
    float climbSpeed = 80.0f;

    bool operator==(const ClimbMoveAbilityData &) const = default;
};