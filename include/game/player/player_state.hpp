#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"
#include "game/player/player_animation_state.hpp"

struct PlayerState
{
     glm::vec2 size = glm::vec2(16.0f);
     bool facingLeft = false;

     glm::vec2 position = glm::vec2(0.0f),
               previousVelocity = glm::vec2(0.0f),
               velocity = glm::vec2(0.0f),
               colliderSize = glm::vec2(16.0f),
               colliderOffset = glm::vec2(0.0f);
     bool onGround = false,
          hitCeiling = false,
          touchingRightWall = false,
          touchingLeftWall = false,
          wasOnGround = false,
          wasHitCeiling = false,
          wasLastWallLeft = false;
     AABB collisionAABBX, collisionAABBY;

     glm::vec2 currentAnimationUVStart = glm::vec2(0, 0);
     glm::vec2 currentAnimationUVEnd = glm::vec2(1, 1);
     PlayerAnimationState currentAnimationState = PlayerAnimationState::Idle;

     bool dashing = false,
          canDash = true;
     float dashTimeLeft = 0,
           dashDirection = 1;

     bool jumping = false;
     float jumpHoldTime = 0.0f;

     bool wallSliding = false;

     bool wallJumping = false;
     float wallJumpTimeLeft = 0,
           wallJumpDirection = 1;

     bool climbing = false;

     glm::vec2 targetVelocity = glm::vec2(0.0f);
};