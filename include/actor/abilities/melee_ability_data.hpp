#pragma once

#include <string_view>
#include <glm/gtc/matrix_transform.hpp>

inline constexpr std::string_view StrikeCue = "onStrike";
inline constexpr std::string_view RecoverCue = "onRecover";

struct MeleeAbilityData
{
    glm::vec2 reach = glm::vec2(12.0f, 10.0f);
    int damage = 1;

    bool operator==(const MeleeAbilityData &) const = default;
};
