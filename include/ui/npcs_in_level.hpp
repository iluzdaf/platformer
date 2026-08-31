#pragma once

#include <memory>
#include <vector>

class Level;
class Npc;

void drawNpcsInLevel(const Level &level, const std::vector<std::unique_ptr<Npc>> &npcs);
