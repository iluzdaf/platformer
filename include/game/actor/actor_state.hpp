#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "game/actor/actor_animation_state.hpp"

struct ActorState
{
    bool facingLeft = false;
    glm::vec2 currentAnimationUVStart = glm::vec2(0, 0),
              currentAnimationUVEnd = glm::vec2(1, 1);
    ActorAnimationState currentAnimationState = ActorAnimationState::Idle;
};
