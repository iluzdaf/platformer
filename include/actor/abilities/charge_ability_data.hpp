#pragma once

#include <string_view>

inline constexpr std::string_view ChargeAttack = "charge";

struct ChargeAbilityData
{
    float speed = 180.0f;
    int damage = 1;

    bool operator==(const ChargeAbilityData &) const = default;
};
