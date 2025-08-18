#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"

struct AgentState
{
      // Agent
      glm::vec2 size = glm::vec2(16.0f);

      // PhysicsBody
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

      // MoveAbility
      glm::vec2 moveVelocity = glm::vec2(0.0f);

      // DashAbility
      bool dashing = false,
           canDash = true,
           emitDash = false;
      float dashTimeLeft = 0,
            dashDirection = 1;
      glm::vec2 dashVelocity = glm::vec2(0.0f);

      // JumpAbility
      glm::vec2 jumpVelocity = glm::vec2(0.0f);
      bool jumping = false;
      float jumpHoldTime = 0.0f;

      // WallSlidingAbility
      bool wallSliding = false,
           emitWallSliding = false;
      glm::vec2 wallSlideVelocity = glm::vec2(0.0f);

      // WallJumpingAbility
      bool wallJumping = false,
           emitWallJump = false;
      float wallJumpTimeLeft = 0,
            wallJumpDirection = 1;
      glm::vec2 wallJumpVelocity = glm::vec2(0.0f);

      // WallClimbingAbility
      bool climbing = false;
      glm::vec2 climbMoveVelocity = glm::vec2(0.0f);

      // GravityAbility
      glm::vec2 gravityVelocity = glm::vec2(0.0f);

      // MovementSystem
      glm::vec2 targetVelocity = glm::vec2(0.0f);
};