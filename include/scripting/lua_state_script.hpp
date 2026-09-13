#pragma once

#include <string>
#include "actor/behaviors/state_script.hpp"
#include "actor/actor_facts.hpp"
#include "input/input_intentions.hpp"

class LuaScriptSystem;
class Npc;
class RouteWalker;

class LuaStateScript : public StateScript
{
public:
    LuaStateScript(LuaScriptSystem &luaScriptSystem, std::string script, Npc *npc);
    void enter(const std::string &call) override;
    InputIntentions decide(
        const std::string &call,
        RouteWalker &walker,
        const ActorFacts &facts,
        float deltaTime) override;
    void exit(const std::string &call) override;

private:
    LuaScriptSystem &luaScriptSystem;
    std::string script;
    Npc *npc;
};
