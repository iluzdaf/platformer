#pragma once

#include <glm/gtc/matrix_transform.hpp>

class NavigationGraph;

struct ActorBehaviorContext
{
    const NavigationGraph &navigationGraph;
    glm::vec2 worldPosition;
    glm::vec2 colliderSize;

    bool onGround = false;
};
