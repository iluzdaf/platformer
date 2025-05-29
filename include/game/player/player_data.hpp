#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp"
#include "animations/sprite_animation_data.hpp"

struct PlayerData
{
    glm::vec2 startPosition = glm::vec2(0, 0);
    float moveSpeed = 160;
    float jumpSpeed = -320;
    glm::vec2 size = glm::vec2(16, 16);
    SpriteAnimationData idleSpriteAnimationData;
    SpriteAnimationData walkSpriteAnimationData;
    int maxJumpCount = 2;
};