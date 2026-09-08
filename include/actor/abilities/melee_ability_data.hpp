#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct MeleeAbilityData
{
    float windup = 0.08f, active = 0.1f, recovery = 0.12f;
    glm::vec2 reach = glm::vec2(12.0f, 10.0f);
    int damage = 1;

    bool operator==(const MeleeAbilityData &) const = default;
};
