#include <string>
#include "scripting/npc_hooks.hpp"
#include "scripting/lua_script_system.hpp"
#include "game/noise.hpp"
#include "npc/npc.hpp"

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
    npc.onNoise.connect(
        [&luaScriptSystem, script, it](const Noise &noise)
        { luaScriptSystem.emitTo(script, "onNoise", it, it, noise.kind, noise.at); });
}
