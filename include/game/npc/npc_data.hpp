#pragma once

#include <optional>
#include "game/actor/actor_data.hpp"
#include "game/actor/behaviors/patrol_behavior_data.hpp"

struct NpcData
{
    ActorData actorData;

    std::optional<PatrolBehaviorData> patrolBehaviorData;
};
