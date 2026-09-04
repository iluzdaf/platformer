#pragma once

#include <functional>
#include <string>
#include <vector>
#include "game/level_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "ui/renames.hpp"

namespace rewriting
{
    using Rewrite = std::function<bool(LevelData &)>;

    struct Reach
    {
        std::vector<std::string> levels, unreadable;
    };

    bool paletteIn(TileMapData &tileMapData, const Renames &renames);
    bool typeIn(std::vector<NpcSpawnData> &npcs, const Renames &renames);
    bool typeIn(std::vector<PickupSpawnData> &pickups, const Renames &renames);

    Reach theLevels(const std::string &directory, const Rewrite &rewrite);
    Reach whatItWouldReach(const std::string &directory, const Rewrite &rewrite);
}
