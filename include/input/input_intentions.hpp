#pragma once
#include <glm/gtc/matrix_transform.hpp>

struct InputIntentions
{
    glm::vec2 direction = {0, 0};
    bool jumpRequested = false;
    bool jumpHeld = false;
    bool dashRequested = false;
    bool climbRequested = false;
};