#pragma once

#include <optional>
#include <string>
#include "actor/actor_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

struct NpcData
{
    ActorData actorData;

    std::optional<StateMachineBehaviorData> stateMachineBehaviorData;
    int contactDamage = 0;
    std::string script;
};
