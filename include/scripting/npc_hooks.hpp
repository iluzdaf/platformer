#pragma once

#include <string>

class LuaScriptSystem;
class Npc;

std::string scriptOf(const std::string &npcType);

void connectNpcHooks(LuaScriptSystem &luaScriptSystem, Npc &npc);
