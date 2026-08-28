#pragma once

struct MoveAbilityData
{
    float moveSpeed = 250.0f;

    bool operator==(const MoveAbilityData &) const = default;
};
