#include <memory>
#include <vector>
#include "npc/striking_player.hpp"
#include "npc/npc.hpp"
#include "player/player.hpp"

void strikePlayer(Player &player, const std::vector<std::unique_ptr<Npc>> &npcs)
{
    for (const std::unique_ptr<Npc> &npc : npcs)
        if (npc->strike(player))
            return;
}
