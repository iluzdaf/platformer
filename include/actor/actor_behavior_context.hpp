#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_contact_state.hpp"

class NavigationGraph;

struct ActorBehaviorContext
{
    const NavigationGraph &navigationGraph;
    glm::vec2 worldPosition;
    glm::vec2 colliderSize;
    std::optional<glm::vec2> threatPosition;

    ActorContactState contacts;
};
