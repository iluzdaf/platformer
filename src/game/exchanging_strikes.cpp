#include <memory>
#include <vector>
#include "game/exchanging_strikes.hpp"
#include "npc/npc.hpp"
#include "player/player.hpp"

void exchangeStrikes(Player &player, const std::vector<std::unique_ptr<Npc>> &npcs)
{
    for (const std::unique_ptr<Npc> &npc : npcs)
        player.strike(*npc);

    for (const std::unique_ptr<Npc> &npc : npcs)
        if (npc->strike(player))
            return;
}
