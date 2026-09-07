#pragma once

struct KnockbackAbilityData
{
    float speed = 160.0f;
    float lift = -140.0f;
    float duration = 0.15f;

    bool operator==(const KnockbackAbilityData &) const = default;
};
