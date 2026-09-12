#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <string>

struct Appearance
{
    glm::vec2 size = glm::vec2(16.0f);
    int currentFrame = 0;
    std::string currentAnimation;
};
