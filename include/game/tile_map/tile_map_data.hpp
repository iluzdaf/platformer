#pragma once
#include <map>
#include <string>
#include <vector>
#include <optional>
#include "game/tile_map/tile_palette.hpp"
#include "game/npc/npc_spawn_data.hpp"
#include "serialization/glm_ivec2_meta.hpp"

struct TileMapData
{
    int size = 16;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<std::vector<std::vector<int>>> indices;
    std::string tilePalette = "default";
    glm::ivec2 playerStartTilePosition = glm::ivec2(0, 0);
    std::string nextLevel = "../assets/levels/level1.json";
    std::vector<NpcSpawnData> npcs;
};