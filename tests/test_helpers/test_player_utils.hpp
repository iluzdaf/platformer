#pragma once
#include "game/player/player.hpp"
#include "game/player/player_data.hpp"
#include "game/actor/abilities/jump_ability_data.hpp"
#include "game/actor/abilities/dash_ability_data.hpp"
#include "game/actor/abilities/move_ability_data.hpp"
#include "game/actor/abilities/wall_slide_ability_data.hpp"
#include "game/actor/abilities/wall_jump_ability_data.hpp"
#include "game/actor/abilities/climb_ability_data.hpp"
#include "game/actor/abilities/climb_move_ability_data.hpp"
#include "game/actor/abilities/gravity_ability_data.hpp"
#include "animations/sprite_animation_data.hpp"
#include "input/intention_source.hpp"

class ScriptedIntentions : public IntentionSource
{
public:
    void set(const InputIntentions &newIntentions)
    {
        intentions = newIntentions;
    }

    InputIntentions getIntentions() const override
    {
        return intentions;
    }

private:
    InputIntentions intentions;
};

inline const IntentionSource &noIntentions()
{
    static const ScriptedIntentions source;
    return source;
}

inline Player setupPlayer(const IntentionSource &intentionSource = noIntentions())
{
    PlayerData playerData;
    playerData.animationData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.animationData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
    playerData.motionData.moveAbilityData = MoveAbilityData();
    playerData.motionData.jumpAbilityData = JumpAbilityData();
    playerData.motionData.dashAbilityData = DashAbilityData();
    playerData.motionData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.motionData.wallJumpAbilityData = WallJumpAbilityData();
    playerData.motionData.climbAbilityData = ClimbAbilityData();
    playerData.motionData.climbMoveAbilityData = ClimbMoveAbilityData();
    playerData.motionData.gravityAbilityData = GravityAbilityData();
    return Player(playerData, intentionSource);
}