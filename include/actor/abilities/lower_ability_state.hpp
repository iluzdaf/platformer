#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct LowerAbilityState
{
    bool active = false, dropping = false, stillWalkingOff = false;
    float timeLeft = 0.0f, direction = 1.0f;
    glm::vec2 velocity = glm::vec2(0.0f);
};
