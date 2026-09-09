#pragma once

#include <string_view>
#include <glm/gtc/matrix_transform.hpp>

inline constexpr std::string_view PounceAttack = "pounce";

struct PounceAbilityData
{
    glm::vec2 leap = glm::vec2(200.0f, -70.0f);
    int damage = 1;

    bool operator==(const PounceAbilityData &) const = default;
};
