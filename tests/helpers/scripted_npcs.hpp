#pragma once

#include <memory>
#include <string>
#include "assets/asset_paths.hpp"
#include "game/level.hpp"
#include "npc/npc.hpp"
#include "scripting/lua_script_system.hpp"
#include "scripting/npc_hooks.hpp"

class ScriptedNpcs
{
public:
    void script(Npc &npc)
    {
        const std::string &path = npc.builtFrom().script.path;
        if (!path.empty())
            lua.use(scriptOf(npc.type()), assets::pathTo(path));

        connectNpcHooks(lua, npc);
    }

    void script(const Level &level)
    {
        for (const std::unique_ptr<Npc> &npc : level.getNpcs())
            script(*npc);
    }

private:
    LuaScriptSystem lua;
};
