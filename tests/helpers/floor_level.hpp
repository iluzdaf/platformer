#pragma once

#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "helpers/levels.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc_spawn_data.hpp"

inline constexpr int FloorLevelTiles = 10;
inline constexpr int FloorLevelRow = 6;
inline constexpr int FloorLevelStanding = FloorLevelRow - 1;

inline LevelData aFloorLevelPlacing(
    const std::vector<NpcSpawnData> &npcs,
    int floorTile = SolidTile)
{
    Placed laid;
    layRow(laid, FloorLevelRow, 0, FloorLevelTiles - 1, floorTile);
    return aLevelPlacing(
        laid, FloorLevelTiles, FloorLevelTiles, glm::ivec2(1, FloorLevelStanding), npcs);
}
