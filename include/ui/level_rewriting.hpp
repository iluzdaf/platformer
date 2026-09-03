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

    bool paletteIn(TileMapData &tileMapData, const Renames &renames);
    bool typeIn(std::vector<NpcSpawnData> &npcs, const Renames &renames);
    bool typeIn(std::vector<PickupSpawnData> &pickups, const Renames &renames);

    std::vector<std::string> theLevels(const std::string &directory, const Rewrite &rewrite);
    std::vector<std::string> whatItWouldReach(const std::string &directory, const Rewrite &rewrite);
}
