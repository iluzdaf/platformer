#pragma once

#include <glm/gtc/matrix_transform.hpp>

class NavigationGraph;

struct ActorBehaviorContext
{
    const NavigationGraph &navigationGraph;
    glm::vec2 worldPosition;
    glm::vec2 colliderSize;

    // Whether it is standing on something, which is when where it stands can be
    // compared against where the graph says it should be.
    bool onGround = false;
};
