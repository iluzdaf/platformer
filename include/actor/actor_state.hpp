#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_animation_state.hpp"

struct ActorState
{
    glm::vec2 size = glm::vec2(16.0f);
    bool facingLeft = false;
    int currentFrame = 0;
    ActorAnimationState currentAnimationState = ActorAnimationState::Idle;
};
