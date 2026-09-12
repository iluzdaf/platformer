#pragma once

#include <memory>
#include <vector>

class Npc;
class Player;

void exchangeStrikes(Player &player, const std::vector<std::unique_ptr<Npc>> &npcs);
