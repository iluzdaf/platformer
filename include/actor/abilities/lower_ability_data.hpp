#pragma once

struct LowerAbilityData
{
    float lowerSpeed = 90.0f, lowerDuration = 0.3f;

    bool operator==(const LowerAbilityData &) const = default;
};
