#pragma once

#include <string>
#include <vector>
#include "game/tile_map/tile_map_data.hpp"
#include "game/npc/npc_spawn_data.hpp"
#include "serialization/glm_ivec2_meta.hpp" // IWYU pragma: keep

struct LevelData
{
    TileMapData tileMapData;
    glm::ivec2 playerStartTilePosition = glm::ivec2(0, 0);
    std::string nextLevel = "../assets/levels/level1.json";
    std::vector<NpcSpawnData> npcs;
};
