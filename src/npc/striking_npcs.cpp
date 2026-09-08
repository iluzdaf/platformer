#include <memory>
#include <vector>
#include "npc/striking_npcs.hpp"
#include "npc/npc.hpp"
#include "player/player.hpp"

void strikeNpcs(Player &player, const std::vector<std::unique_ptr<Npc>> &npcs)
{
    for (const std::unique_ptr<Npc> &npc : npcs)
        player.strike(*npc);
}
