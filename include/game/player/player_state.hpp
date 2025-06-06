#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "physics/aabb.hpp"

struct PlayerState
{
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 velocity = glm::vec2(0.0f);
    glm::vec2 colliderSize = glm::vec2(16.0f);
    bool onGround = false;
    bool touchingRightWall = false;
    bool touchingLeftWall = false;
    AABB collisionAABBX;
    AABB collisionAABBY;

    glm::vec2 size = glm::vec2(16.0f);

    bool facingLeft = false;

    bool dashing = false;
    float dashDuration = 0.2f;

    int maxJumpCount = 2;
    float jumpSpeed = -280.0f;

    float moveSpeed = 160.0f;
    
    bool wallSliding = false;

    bool wallJumping = false;
};