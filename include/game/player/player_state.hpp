#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"
#include "game/player/player_animation_state.hpp"

struct PlayerState
{
    glm::vec2 position = glm::vec2(0.0f),
              previousVelocity = glm::vec2(0.0f),
              velocity = glm::vec2(0.0f),
              colliderSize = glm::vec2(16.0f);
    bool onGround = false,
         hitCeiling = false,
         touchingRightWall = false,
         touchingLeftWall = false,
         wasOnGround = false,
         wasHitCeiling = false;
    AABB collisionAABBX, collisionAABBY;

    glm::vec2 size = glm::vec2(16.0f);
    bool facingLeft = false;

    glm::vec2 currentAnimationUVStart = glm::vec2(0, 0);
    glm::vec2 currentAnimationUVEnd = glm::vec2(1, 1);
    PlayerAnimationState currentAnimationState = PlayerAnimationState::Idle;

    bool dashing = false;
    float dashDuration = 0.2f;

    int maxJumpCount = 2;
    float jumpSpeed = -280.0f;

    float moveSpeed = 160.0f;

    bool wallSliding = false;

    bool wallJumping = false;
};