#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct MantleAbilityState
{
    bool active = false;
    float timeLeft = 0.0f, direction = 1.0f;
    glm::vec2 velocity = glm::vec2(0.0f);
};
