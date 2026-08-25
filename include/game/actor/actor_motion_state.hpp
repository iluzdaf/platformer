#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "game/actor/actor_contact_state.hpp"

struct MoveState
{
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct DashState
{
    bool active = false,
         available = true,
         emit = false;
    float timeLeft = 0,
          direction = 1;
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct JumpState
{
    bool active = false;
    float holdTime = 0.0f;
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct WallSlideState
{
    bool active = false,
         emit = false;
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct WallJumpState
{
    bool active = false,
         emit = false;
    float timeLeft = 0,
          direction = 1;
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct ClimbState
{
    bool active = false;
};

struct ClimbMoveState
{
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct GravityState
{
    glm::vec2 velocity = glm::vec2(0.0f);
};

struct ActorMotionState
{
    ActorContactState contacts;

    glm::vec2 previousVelocity = glm::vec2(0.0f),
              velocity = glm::vec2(0.0f);

    MoveState move;
    DashState dash;
    JumpState jump;
    WallSlideState wallSlide;
    WallJumpState wallJump;
    ClimbState climb;
    ClimbMoveState climbMove;
    GravityState gravity;

    glm::vec2 targetVelocity = glm::vec2(0.0f);
};
