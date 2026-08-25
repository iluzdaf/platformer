#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct JumpAbilityState
{
    bool active = false;
    float holdTime = 0.0f;
    glm::vec2 velocity = glm::vec2(0.0f);
};
