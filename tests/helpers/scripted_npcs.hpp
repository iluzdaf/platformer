#pragma once

#include "assets/asset_paths.hpp"
#include "helpers/shipped.hpp"
#include "npc/npc.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/npc_hooks.hpp"

class ScriptedNpcs
{
public:
    void script(Npc &npc)
    {
        lua.use(scriptOf(npc.type()), assets::pathTo(shippedNpcData().at(npc.type()).script.path));
        connectNpcHooks(lua, npc);
    }

private:
    LuaScriptSystem lua;
};
