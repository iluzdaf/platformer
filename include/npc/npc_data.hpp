#pragma once

#include <optional>
#include "actor/actor_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"

struct NpcData
{
    ActorData actorData;

    std::optional<PatrolBehaviorData> patrolBehaviorData;
};
