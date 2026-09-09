#pragma once

#include <memory>
#include <vector>

class Player;
class Npc;

void strikePlayer(Player &player, const std::vector<std::unique_ptr<Npc>> &npcs);
