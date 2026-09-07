#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>

struct KnockbackAbilityState
{
    bool active = false, emit = false;
    float timeLeft = 0, direction = 1;
    glm::vec2 velocity = glm::vec2(0.0f);
    std::optional<glm::vec2> pushed;
};
