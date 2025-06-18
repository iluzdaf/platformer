#pragma once
#include "game/player/player.hpp"
#include "game/player/player_data.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "physics/physics_data.hpp"
#include "animations/sprite_animation_data.hpp"

inline Player setupPlayer(
    float gravity = 980.0f,
    float moveSpeed = 160.0f)
{
    PlayerData playerData;
    playerData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
    playerData.moveAbilityData = MoveAbilityData();
    playerData.moveAbilityData->moveSpeed = moveSpeed;
    playerData.jumpAbilityData = JumpAbilityData();
    playerData.dashAbilityData = DashAbilityData();
    playerData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.wallJumpAbilityData = WallJumpAbilityData();
    PhysicsData physicsData;
    physicsData.gravity = gravity;
    return Player(playerData, physicsData);
}