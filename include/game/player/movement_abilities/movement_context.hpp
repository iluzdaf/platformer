#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "input/input_intentions.hpp"

struct MovementContext
{
    glm::vec2 velocity = {0, 0},
              moveVelocity = {0, 0},
              jumpVelocity = {0, 0},
              dashVelocity = {0, 0},
              climbMoveVelocity = {0, 0},
              wallSlideVelocity = {0, 0},
              wallJumpVelocity = {0, 0};
    InputIntentions inputIntentions;
    bool emitWallJump = false,
         emitDash = false,
         emitWallSliding = false;
};