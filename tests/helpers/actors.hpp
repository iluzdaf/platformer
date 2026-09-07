#pragma once

#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/health_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/level.hpp"
#include "input/input_intentions.hpp"
#include "input/intention_source.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"

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

inline PlayerData playerDataWithEveryAbility()
{
    PlayerData playerData;
    playerData.actorData.animationData.idle = FrameAnimationData({0}, 1.0f);
    playerData.actorData.animationData.walk = FrameAnimationData({1, 2, 3}, 0.1f);
    playerData.actorData.motionData.moveAbilityData = MoveAbilityData();
    playerData.actorData.motionData.jumpAbilityData = JumpAbilityData();
    playerData.actorData.motionData.dashAbilityData = DashAbilityData();
    playerData.actorData.motionData.wallSlideAbilityData = WallSlideAbilityData();
    playerData.actorData.motionData.wallJumpAbilityData = WallJumpAbilityData();
    playerData.actorData.motionData.wallHangAbilityData = WallHangAbilityData();
    playerData.actorData.motionData.wallClimbAbilityData = WallClimbAbilityData();
    playerData.actorData.motionData.mantleAbilityData = MantleAbilityData();
    playerData.actorData.motionData.gravityAbilityData = GravityAbilityData();
    playerData.actorData.motionData.knockbackAbilityData = KnockbackAbilityData();
    return playerData;
}

inline PlayerData playerDataWithHealth(int points, float invulnerableFor)
{
    PlayerData playerData = playerDataWithEveryAbility();
    playerData.actorData.healthData = HealthData{points, invulnerableFor};
    return playerData;
}

inline Player aPlayerWithEveryAbility(const IntentionSource &intentionSource = noIntentions())
{
    return Player(playerDataWithEveryAbility(), intentionSource);
}

inline std::vector<NpcSpawnData> spawnsIn(const Level &level)
{
    std::vector<NpcSpawnData> spawns;
    for (const std::unique_ptr<Npc> &npc : level.getNpcs())
        spawns.push_back(npc->getSpawn());

    return spawns;
}

inline ActorData anActorOfHeight(float height)
{
    ActorData actorData;
    actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, height);
    return actorData;
}

inline ActorMotionData jumperMotionData()
{
    ActorMotionData motionData;
    motionData.moveAbilityData = MoveAbilityData{};
    motionData.gravityAbilityData = GravityAbilityData{};
    motionData.jumpAbilityData = JumpAbilityData{};
    return motionData;
}

inline ActorMotionData fallerMotionData()
{
    ActorMotionData motionData;
    motionData.gravityAbilityData = GravityAbilityData{};
    return motionData;
}

inline ActorMotionData climberMotionData()
{
    ActorMotionData motionData;
    motionData.wallHangAbilityData = WallHangAbilityData();
    motionData.wallClimbAbilityData = WallClimbAbilityData();
    return motionData;
}

inline NavigationProfile profileThatMoves(float height, const ActorMotionData &motionData)
{
    ActorData actorData = anActorOfHeight(height);
    actorData.motionData = motionData;
    return buildNavigationProfile(actorData);
}

inline NavigationProfile profileOfHeight(float height)
{
    return buildNavigationProfile(anActorOfHeight(height));
}

inline NavigationProfile standardProfile()
{
    return profileOfHeight(13.0f);
}

inline NavigationProfile jumperProfile()
{
    return profileThatMoves(13.0f, jumperMotionData());
}

inline NavigationProfile climberProfile()
{
    return profileThatMoves(13.0f, climberMotionData());
}
