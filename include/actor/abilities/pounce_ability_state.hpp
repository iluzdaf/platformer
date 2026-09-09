#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct PounceAbilityState
{
    bool active = false, emit = false, leftTheGround = false;
    float direction = 1;
    glm::vec2 velocity = glm::vec2(0.0f);
    int damage = 0;
};
