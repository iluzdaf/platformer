#pragma once
#include <glm/gtc/matrix_transform.hpp>

struct PlayerState
{
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 velocity = glm::vec2(0.0f);
    glm::vec2 size = glm::vec2(1.0f);
    bool onGround = false;
    bool facingLeft = false;
    bool dashing = false;
    int maxJumpCount = 2;
    float jumpSpeed = -280.0f;
    float moveSpeed = 160.0f;
    float dashDuration = 0.2f;
    bool wallSliding = false;
    bool touchingRightWall = false;
    bool touchingLeftWall = false;
};