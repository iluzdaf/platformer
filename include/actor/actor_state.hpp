#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <string>

struct ActorState
{
    glm::vec2 size = glm::vec2(16.0f);
    bool facingLeft = false;
    int currentFrame = 0;
    std::string currentAnimation = "idle";
};
