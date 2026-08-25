#pragma once

#include <optional>
#include "agent/agent_data.hpp"
#include "game/actor/actor_animation_data.hpp"
#include "game/npc/behaviors/patrol_behavior_data.hpp"

struct NpcData
{
    AgentData agentData;
    ActorAnimationData animationData;

    std::optional<PatrolBehaviorData> patrolBehaviorData;
};
