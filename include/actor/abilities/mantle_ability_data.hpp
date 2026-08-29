#pragma once

struct MantleAbilityData
{
    float mantleSpeed = 90.0f, mantleDuration = 0.3f;

    bool operator==(const MantleAbilityData &) const = default;
};
