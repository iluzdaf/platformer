#pragma once

struct WallClimbAbilityData
{
    float climbSpeed = 80.0f;

    bool operator==(const WallClimbAbilityData &) const = default;
};