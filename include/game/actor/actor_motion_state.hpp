#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "game/actor/actor_contact_state.hpp"

struct ActorMotionState
{
    ActorContactState contacts;

    glm::vec2 previousVelocity = glm::vec2(0.0f),
              velocity = glm::vec2(0.0f);

    glm::vec2 moveVelocity = glm::vec2(0.0f);

    bool dashing = false,
         canDash = true,
         emitDash = false;
    float dashTimeLeft = 0,
          dashDirection = 1;
    glm::vec2 dashVelocity = glm::vec2(0.0f);

    glm::vec2 jumpVelocity = glm::vec2(0.0f);
    bool jumping = false;
    float jumpHoldTime = 0.0f;

    bool wallSliding = false,
         emitWallSliding = false;
    glm::vec2 wallSlideVelocity = glm::vec2(0.0f);

    bool wallJumping = false,
         emitWallJump = false;
    float wallJumpTimeLeft = 0,
          wallJumpDirection = 1;
    glm::vec2 wallJumpVelocity = glm::vec2(0.0f);

    bool climbing = false;
    glm::vec2 climbMoveVelocity = glm::vec2(0.0f);

    glm::vec2 gravityVelocity = glm::vec2(0.0f);

    glm::vec2 targetVelocity = glm::vec2(0.0f);
};
