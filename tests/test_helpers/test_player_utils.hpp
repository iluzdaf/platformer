#pragma once
#include "game/player/player.hpp"
#include "game/player/player_data.hpp"
#include "agent/movement_abilities/jump_ability_data.hpp"
#include "agent/movement_abilities/dash_ability_data.hpp"
#include "agent/movement_abilities/move_ability_data.hpp"
#include "agent/movement_abilities/wall_slide_ability_data.hpp"
#include "agent/movement_abilities/wall_jump_ability_data.hpp"
#include "agent/movement_abilities/climb_ability_data.hpp"
#include "agent/movement_abilities/climb_move_ability_data.hpp"
#include "agent/movement_abilities/gravity_ability_data.hpp"
#include "animations/sprite_animation_data.hpp"

inline Player setupPlayer()
{
    PlayerData playerData;
    playerData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
    playerData.agentData.moveAbilityData = MoveAbilityData();
    playerData.agentData.jumpAbilityData = JumpAbilityData();
    playerData.agentData.dashAbilityData = DashAbilityData();
    playerData.agentData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.agentData.wallJumpAbilityData = WallJumpAbilityData();
    playerData.agentData.climbAbilityData = ClimbAbilityData();
    playerData.agentData.climbMoveAbilityData = ClimbMoveAbilityData();
    playerData.agentData.gravityAbilityData = GravityAbilityData();
    return Player(playerData);
}