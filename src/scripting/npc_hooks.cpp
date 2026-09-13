#include <string>
#include "scripting/npc_hooks.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/lua_state_script.hpp"
#include <memory>
#include <optional>
#include <set>
#include <variant>
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "npc/npc_data.hpp"
#include "game/noise.hpp"
#include "npc/npc.hpp"

namespace
{
    std::set<std::string> statesScriptedFor(const Npc &npc)
    {
        std::set<std::string> calls;
        if (const std::optional<StateMachineBehaviorData> &machine =
                npc.builtFrom().stateMachineBehaviorData)
            for (const BehaviorStateData &state : machine->states)
                if (const auto *scripted = std::get_if<ScriptedBehaviorData>(&state.does))
                    calls.insert(scripted->call);

        return calls;
    }
}

std::string scriptOf(const std::string &npcType)
{
    return "npc:" + npcType;
}

void connectNpcHooks(LuaScriptSystem &luaScriptSystem, Npc &npc)
{
    Npc *it = &npc;
    std::string script = scriptOf(npc.type());
    npc.onHurt.connect([&luaScriptSystem, script, it]
                       { luaScriptSystem.emitTo(script, "onHurt", it, it); });
    npc.onDeath.connect([&luaScriptSystem, script, it]
                        { luaScriptSystem.emitTo(script, "onDeath", it, it); });
    npc.onCue.connect([&luaScriptSystem, script, it](const std::string &cue)
                      { luaScriptSystem.emitTo(script, cue, it, it); });
    npc.onTick.connect([&luaScriptSystem, script, it](float deltaTime)
                       { luaScriptSystem.emitTo(script, "onTick", it, it, deltaTime); });
    npc.scriptStatesWith(std::make_unique<LuaStateScript>(luaScriptSystem, script, &npc));
    luaScriptSystem.expectStates(script, statesScriptedFor(npc));
    npc.onNoise.connect(
        [&luaScriptSystem, script, it](const Noise &noise)
        { luaScriptSystem.emitTo(script, "onNoise", it, it, noise.kind, noise.at); });
}
