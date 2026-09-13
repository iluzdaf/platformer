#pragma once

#include <filesystem>
#include <map>
#include <utility>
#include <optional>
#include <string>
#include "actor/actor_facts.hpp"
#include "actor/behaviors/patrol_data.hpp"
#include "actor/behaviors/scripted_behavior.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "helpers/creature_scripts.hpp"
#include "input/input_intentions.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/lua_state_script.hpp"

class BehaviorFile
{
public:
    explicit BehaviorFile(
        const std::string &path,
        std::optional<PatrolData> beat = std::nullopt,
        std::map<std::string, float> tuning = {})
        : script(lua, "tested", nullptr),
          behavior(ScriptedBehaviorData{std::filesystem::path(path).stem().string()}), beat(beat),
          tuning(std::move(tuning))
    {
        lua.use("tested", aScriptThatRuns(path));
        behavior.scriptWith(&script);
    }

    InputIntentions decide(float deltaTime, ActorFacts facts)
    {
        facts.beat = beat ? &*beat : nullptr;
        facts.tuning = &tuning;
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
    std::map<std::string, float> tuning;
};
