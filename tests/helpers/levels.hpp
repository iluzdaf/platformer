#pragma once

#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc_spawn_data.hpp"
#include "tile_map/tile_map_data.hpp"

inline constexpr int FloorLevelTiles = 10;
inline constexpr int FloorLevelRow = 6;
inline constexpr int FloorLevelStanding = FloorLevelRow - 1;

inline NpcSpawnData spawnAt(std::string type, glm::ivec2 tilePosition)
{
    NpcSpawnData spawn;
    spawn.type = std::move(type);
    spawn.position = feetOf(tilePosition);
    return spawn;
}

inline NpcSpawnData aVillagerAt(glm::ivec2 tilePosition)
{
    return spawnAt("villager", tilePosition);
}

inline LevelData aFloorLevelPlacing(const std::vector<NpcSpawnData> &npcs, int floorTile = 1)
{
    LevelData levelData;
    levelData.tileMapData.tilePalette = "default";
    levelData.tileMapData.indices =
        std::vector<std::vector<int>>(FloorLevelTiles, std::vector<int>(FloorLevelTiles, 0));
    for (int x = 0; x < FloorLevelTiles; ++x)
        levelData.tileMapData.indices[FloorLevelRow][x] = floorTile;

    levelData.playerStart = feetOf(glm::ivec2(1, FloorLevelStanding));
    levelData.npcs = npcs;
    return levelData;
}
