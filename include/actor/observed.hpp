#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_contact_state.hpp"
#include "combat/hit.hpp"

struct Observed
{
    ActorContactState contacts;
    std::vector<Hit> hits;
    glm::vec2 velocity = glm::vec2(0.0f), previousVelocity = glm::vec2(0.0f);
    float fell = 0.0f;
    bool facingLeft = false, alive = true;
};
