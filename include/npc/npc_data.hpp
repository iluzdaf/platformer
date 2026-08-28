#pragma once

#include <optional>
#include "actor/actor_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

struct NpcData
{
    ActorData actorData;

    std::optional<PatrolBehaviorData> patrolBehaviorData;
    std::optional<StateMachineBehaviorData> stateMachineBehaviorData;
};
