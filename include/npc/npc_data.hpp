#pragma once

#include <map>
#include <optional>
#include <string>
#include "actor/actor_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "conditions/facts.hpp"

struct NpcData
{
    ActorData actorData;

    std::optional<StateMachineBehaviorData> stateMachineBehaviorData;
    FactsData facts;
    std::map<std::string, float> tuning;
    int contactDamage = 0;
    std::string script;
};
