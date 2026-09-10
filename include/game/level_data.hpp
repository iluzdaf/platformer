#pragma once

#include <vector>
#include "game/level_path_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep

struct LevelData
{
    TileMapData tileMapData;
    glm::vec2 playerFeet = glm::vec2(0.0f);
    LevelPathData nextLevel = "../assets/levels/level1.json";
    std::vector<NpcSpawnData> npcs;
    std::vector<PickupSpawnData> pickups;
};
