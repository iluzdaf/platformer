#pragma once

#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/dash_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/knockback_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include <string>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/wall_jump_ability_data.hpp"
#include "actor/abilities/wall_slide_ability_data.hpp"
#include "actor/actor_data.hpp"
#include "combat/health_data.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "helpers/rules.hpp"
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

inline FrameAnimationData anAttackClip()
{
    FrameAnimationData clip{{12, 13, 14}, 0.1f};
    clip.loops = false;
    return clip;
}

inline PlayerData playerDataWithEveryAbility()
{
    PlayerData playerData;

    playerData.actorData.abilities.move = MoveAbilityData();
    playerData.actorData.abilities.jump = JumpAbilityData();
    playerData.actorData.abilities.dash = DashAbilityData();
    playerData.actorData.abilities.wallSlide = WallSlideAbilityData();
    playerData.actorData.abilities.wallJump = WallJumpAbilityData();
    playerData.actorData.abilities.wallHang = WallHangAbilityData();
    playerData.actorData.abilities.wallClimb = WallClimbAbilityData();
    playerData.actorData.abilities.mantle = MantleAbilityData();
    playerData.actorData.abilities.gravity = GravityAbilityData();
    playerData.actorData.abilities.knockback = KnockbackAbilityData();
    playerData.actorData.abilities.swing = SwingAbilityData();

    AnimatorData &animations = playerData.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    animations.clips["walk"] = FrameAnimationData({1, 2, 3}, 0.1f);
    animations.clips["attack"] = anAttackClip();
    animations.clips["dead"] = FrameAnimationData({9}, 1.0f);
    animations.rules = {deadRule(), swingRule(), walkRule(), idleRule()};

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

inline AbilitiesData jumperAbilities()
{
    AbilitiesData abilitiesData;
    abilitiesData.move = MoveAbilityData{};
    abilitiesData.gravity = GravityAbilityData{};
    abilitiesData.jump = JumpAbilityData{};
    return abilitiesData;
}

inline AbilitiesData fallerAbilities()
{
    AbilitiesData abilitiesData;
    abilitiesData.gravity = GravityAbilityData{};
    return abilitiesData;
}

inline AbilitiesData climberAbilities()
{
    AbilitiesData abilitiesData;
    abilitiesData.wallHang = WallHangAbilityData();
    abilitiesData.wallClimb = WallClimbAbilityData();
    return abilitiesData;
}

inline NavigationProfile profileThatMoves(float height, const AbilitiesData &abilitiesData)
{
    ActorData actorData = anActorOfHeight(height);
    actorData.abilities = abilitiesData;
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
    return profileThatMoves(13.0f, jumperAbilities());
}

inline NavigationProfile climberProfile()
{
    return profileThatMoves(13.0f, climberAbilities());
}
