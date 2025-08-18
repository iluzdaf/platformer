#pragma once

#include "game/player/player_animation_state.hpp"

struct PlayerState
{
     bool facingLeft = false;
     glm::vec2 currentAnimationUVStart = glm::vec2(0, 0),
               currentAnimationUVEnd = glm::vec2(1, 1);
     PlayerAnimationState currentAnimationState = PlayerAnimationState::Idle;
};