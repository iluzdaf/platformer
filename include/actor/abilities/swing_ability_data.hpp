#pragma once

#include <string_view>
#include <glm/gtc/matrix_transform.hpp>

inline constexpr std::string_view SwingAttack = "swing";

struct SwingAbilityData
{
    glm::vec2 reach = glm::vec2(12.0f, 10.0f);
    int damage = 1;
    float windupDuration = 0.1f;
    float strikeDuration = 0.1f;
    float recoveryDuration = 0.1f;

    bool operator==(const SwingAbilityData &) const = default;
};
