#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_animation_state.hpp"

struct ActorState
{
    glm::vec2 size = glm::vec2(16.0f);
    bool facingLeft = false;
    glm::vec2 currentAnimationUVStart = glm::vec2(0, 0),
              currentAnimationUVEnd = glm::vec2(1, 1);
    ActorAnimationState currentAnimationState = ActorAnimationState::Idle;
};
