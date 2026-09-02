#pragma once
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "input/intention_source.hpp"
#include "input/input_intentions.hpp"
#include "animations/frame_animation_data.hpp"

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
    playerData.actorData.animationData.idle = FrameAnimationData({30}, 1.0f);
    playerData.actorData.animationData.walk = FrameAnimationData({34, 26, 35}, 0.1f);
    playerData.actorData.motionData.moveAbilityData = MoveAbilityData();
    playerData.actorData.motionData.jumpAbilityData = JumpAbilityData();
    playerData.actorData.motionData.dashAbilityData = DashAbilityData();
    playerData.actorData.motionData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.actorData.motionData.wallJumpAbilityData = WallJumpAbilityData();
    playerData.actorData.motionData.wallHangAbilityData = WallHangAbilityData();
    playerData.actorData.motionData.wallClimbAbilityData = WallClimbAbilityData();
    playerData.actorData.motionData.mantleAbilityData = MantleAbilityData();
    playerData.actorData.motionData.gravityAbilityData = GravityAbilityData();
    return playerData;
}

inline Player setupPlayer(const IntentionSource &intentionSource = noIntentions())
{
    return Player(setupPlayerData(), intentionSource);
}