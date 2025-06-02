#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "serialization/glm_vec2_meta.hpp"
#include "animations/sprite_animation_data.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"

struct PlayerData
{
    glm::vec2 startPosition = glm::vec2(0, 0);
    glm::vec2 size = glm::vec2(16, 16);
    SpriteAnimationData idleSpriteAnimationData;
    SpriteAnimationData walkSpriteAnimationData;
    std::optional<MoveAbilityData> moveAbilityData;
    std::optional<JumpAbilityData> jumpAbilityData;
    std::optional<DashAbilityData> dashAbilityData;
    std::optional<WallSlideAbilityData> wallSlideAbilityData;
    std::optional<WallJumpAbilityData> wallJumpAbilityData;
};