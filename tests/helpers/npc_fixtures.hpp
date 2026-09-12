#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "animations/animation_ladder_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/level.hpp"
#include "helpers/ladders.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"

inline constexpr glm::ivec2 SpawnTile{4, 5};

inline NpcData setupNpcData()
{
    NpcData npcData;

    npcData.actorData.size = glm::vec2(16.0f);

    npcData.actorData.motionData.moveAbilityData = MoveAbilityData{60.0f};
    npcData.actorData.motionData.gravityAbilityData = GravityAbilityData{};

    npcData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
    npcData.actorData.physicsBodyData.colliderOffset = glm::vec2(4.0f, 3.0f);

    BehaviorStateData patrolling;
    patrolling.name = "patrol";
    patrolling.does = PatrolBehaviorData();

    npcData.stateMachineBehaviorData = StateMachineBehaviorData{{patrolling}, {}};

    return npcData;
}

inline void stepNpc(Npc &npc, const Level &level, int steps)
{
    for (int step = 0; step < steps; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
    }
}

inline glm::vec2 footOf(const Npc &npc)
{
    return npc.body().aabb().bottomCenter();
}

inline void noticingAThreatWithin(Npc &npc, float range)
{
    npc.onTick.connect(
        [&npc, range](float)
        {
            std::optional<glm::vec2> threat = npc.threatFeet();
            npc.fact(
                "threatNear",
                threat.has_value() && npc.onSameSurfaceAs(*threat) &&
                    npc.distanceTo(*threat) <= range);
        });
}
