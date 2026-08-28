#pragma once

#include <optional>
#include "actor/actor_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

struct NpcData
{
    ActorData actorData;

    std::optional<StateMachineBehaviorData> stateMachineBehaviorData;
};
