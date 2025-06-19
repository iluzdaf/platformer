#pragma once
#include "game/player/player.hpp"
#include "game/player/player_data.hpp"
#include "game/player/movement_abilities/jump_ability_data.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"
#include "game/player/movement_abilities/wall_slide_ability_data.hpp"
#include "game/player/movement_abilities/wall_jump_ability_data.hpp"
#include "game/player/movement_abilities/climb_ability.hpp"
#include "game/player/movement_abilities/climb_move_ability.hpp"
#include "physics/physics_data.hpp"
#include "animations/sprite_animation_data.hpp"

inline Player setupPlayer()
{
    PlayerData playerData;
    playerData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
    playerData.moveAbilityData = MoveAbilityData();
    playerData.jumpAbilityData = JumpAbilityData();
    playerData.dashAbilityData = DashAbilityData();
    playerData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.wallJumpAbilityData = WallJumpAbilityData();
    playerData.climbAbilityData = ClimbAbilityData();
    playerData.climbMoveAbilityData = ClimbMoveAbilityData();
    PhysicsData physicsData;
    return Player(playerData, physicsData);
}