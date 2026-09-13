#include <string>
#include <utility>
#include <sol/sol.hpp>
#include "scripting/lua_state_script.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/script_walker.hpp"
#include "actor/actor_facts.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "input/input_intentions.hpp"
#include "npc/npc.hpp"

LuaStateScript::LuaStateScript(LuaScriptSystem &luaScriptSystem, std::string script, Npc *npc)
    : luaScriptSystem(luaScriptSystem), script(std::move(script)), npc(npc)
{
}

void LuaStateScript::enter(const std::string &call)
{
    luaScriptSystem.startStateAfresh(npc, call);
    luaScriptSystem.callState(script, npc, call, "enter", npc);
}

InputIntentions LuaStateScript::decide(
    const std::string &call,
    RouteWalker &walker,
    const ActorFacts &facts,
    float deltaTime)
{
    ScriptWalker lent(walker, facts);
    sol::object wanted =
        luaScriptSystem.callState(script, npc, call, "decide", npc, &facts, &lent, deltaTime);

    return wanted.is<InputIntentions>() ? wanted.as<InputIntentions>() : InputIntentions();
}

void LuaStateScript::exit(const std::string &call)
{
    luaScriptSystem.callState(script, npc, call, "exit", npc);
}
