#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct PhysicsBodyData
{
    glm::vec2 colliderSize = glm::vec2(8, 16);
    glm::vec2 colliderOffset = glm::vec2(4, 0);
};