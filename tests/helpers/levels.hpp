#pragma once

#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/beat_between.hpp"
#include "game/level_data.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc_spawn_data.hpp"

inline PatrolData beatOf(glm::ivec2 fromTile, glm::ivec2 toTile)
{
    return beatBetween(fromTile, toTile, TestTileSize);
}

inline NpcSpawnData spawnAt(std::string type, glm::ivec2 tilePosition)
{
    NpcSpawnData spawn;
    spawn.type = std::move(type);
    spawn.feet = feetOf(tilePosition);
    return spawn;
}

inline NpcSpawnData aRatAt(glm::ivec2 tilePosition)
{
    return spawnAt("rat", tilePosition);
}

inline NpcSpawnData patrolling(
    std::string type,
    glm::ivec2 tilePosition,
    glm::ivec2 from,
    glm::ivec2 to)
{
    NpcSpawnData spawn = spawnAt(std::move(type), tilePosition);
    spawn.patrol = beatOf(from, to);
    return spawn;
}

inline LevelData aLevelPlacing(
    const Placed &tiles,
    int width,
    int height,
    glm::ivec2 playerTile,
    const std::vector<NpcSpawnData> &npcs)
{
    LevelData levelData;
    levelData.tileMapData = aTileMapData(tiles, width, height);
    levelData.playerFeet = feetOf(playerTile);
    levelData.npcs = npcs;
    return levelData;
}
