#pragma once

struct BiteAbilityData
{
    int damage = 1;

    bool operator==(const BiteAbilityData &) const = default;
};
