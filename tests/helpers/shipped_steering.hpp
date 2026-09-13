#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include "actor/actor_facts.hpp"
#include "actor/behaviors/patrol_data.hpp"
#include "actor/behaviors/scripted_behavior.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "assets/asset_paths.hpp"
#include "helpers/shipped.hpp"
#include "input/input_intentions.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/lua_state_script.hpp"

inline ScriptedBehaviorData shippedScriptedState(
    const std::string &creature,
    const std::string &state)
{
    for (const BehaviorStateData &each :
         shippedNpcData().at(creature).stateMachineBehaviorData->states)
        if (each.name == state)
            return std::get<ScriptedBehaviorData>(each.does);

    throw std::runtime_error("the shipped " + creature + " has no state called " + state);
}

class ShippedSteering
{
public:
    ShippedSteering(
        const std::string &creature,
        const std::string &state,
        std::optional<PatrolData> beat = std::nullopt)
        : script(lua, creature, nullptr), behavior(shippedScriptedState(creature, state)),
          beat(beat)
    {
        lua.use(creature, assets::pathTo(shippedNpcData().at(creature).script.path));
        behavior.scriptWith(&script);
    }

    InputIntentions decide(float deltaTime, ActorFacts facts)
    {
        facts.beat = beat ? &*beat : nullptr;
        return behavior.decide(deltaTime, facts);
    }

    void reset()
    {
        behavior.reset();
    }

    std::optional<int> getCurrentNodeId() const
    {
        return behavior.getCurrentNodeId();
    }

    std::optional<int> getTargetNodeId() const
    {
        return behavior.getTargetNodeId();
    }

    int errorsReported() const
    {
        return lua.errorsReported();
    }

private:
    LuaScriptSystem lua;
    LuaStateScript script;
    ScriptedBehavior behavior;
    std::optional<PatrolData> beat;
};
