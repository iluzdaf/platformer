#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct WallSlideAbilityState
{
    bool active = false,
         emit = false;
    glm::vec2 velocity = glm::vec2(0.0f);
};
