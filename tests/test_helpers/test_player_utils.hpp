#pragma once
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/climb_ability_data.hpp"
#include "actor/abilities/climb_move_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
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

inline PlayerData setupPlayerData()
{
    PlayerData playerData;
    playerData.actorData.animationData.idleSpriteAnimationData = SpriteAnimationData(FrameAnimationData({30}, 1.0f), 16, 16, 96);
    playerData.actorData.animationData.walkSpriteAnimationData = SpriteAnimationData(FrameAnimationData({34, 26, 35}, 0.1f), 16, 16, 96);
    playerData.actorData.motionData.moveAbilityData = MoveAbilityData();
    playerData.actorData.motionData.jumpAbilityData = JumpAbilityData();
    playerData.actorData.motionData.dashAbilityData = DashAbilityData();
    playerData.actorData.motionData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.actorData.motionData.wallJumpAbilityData = WallJumpAbilityData();
    playerData.actorData.motionData.climbAbilityData = ClimbAbilityData();
    playerData.actorData.motionData.climbMoveAbilityData = ClimbMoveAbilityData();
    playerData.actorData.motionData.gravityAbilityData = GravityAbilityData();
    return playerData;
}

inline Player setupPlayer(const IntentionSource &intentionSource = noIntentions())
{
    return Player(setupPlayerData(), intentionSource);
}