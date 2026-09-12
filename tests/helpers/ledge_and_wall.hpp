#pragma once

#include <map>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "helpers/levels.hpp"
#include "helpers/palettes.hpp"
#include "helpers/shipped.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

namespace ledge_and_wall
{
    inline constexpr int LedgeWidthTiles = 20;
    inline constexpr int LedgeHeightTiles = 14;
    inline constexpr int GroundRow = 12;
    inline constexpr int LedgeRow = 6;
    inline constexpr int LedgeLastTile = 6;
    inline constexpr int StepRow = 8;
    inline constexpr int StepFirstTile = 3;
    inline constexpr int StepLastTile = 9;
    inline constexpr int RiseRow = 10;
    inline constexpr int RiseFirstTile = 12;
    inline constexpr int RiseLastTile = 18;

    inline constexpr glm::ivec2 LedgeLeftEnd{1, LedgeRow - 1};
    inline constexpr glm::ivec2 LedgeRightEnd{LedgeLastTile, LedgeRow - 1};
    inline constexpr glm::ivec2 OnTheGround{2, GroundRow - 1};
    inline constexpr glm::ivec2 TopOfTheWall{1, 0};
}

inline Level levelWithALedgeAndAWall(
    const std::vector<NpcSpawnData> &npcs,
    const std::map<std::string, NpcData> &npcData)
{
    using namespace ledge_and_wall;

    Placed laid;
    layColumn(laid, 0, 0, GroundRow - 1);
    layColumn(laid, LedgeWidthTiles - 1, 0, GroundRow - 1);
    layRow(laid, GroundRow, 1, LedgeWidthTiles - 2, SlipperyTile);
    laid.push_back({glm::ivec2(0, GroundRow), SolidTile});
    laid.push_back({glm::ivec2(LedgeWidthTiles - 1, GroundRow), SolidTile});
    layRow(laid, GroundRow + 1, 0, LedgeWidthTiles - 1);
    layRow(laid, LedgeRow, 1, LedgeLastTile, SlipperyTile);
    layRow(laid, StepRow, StepFirstTile, StepLastTile, SlipperyTile);
    layRow(laid, RiseRow, RiseFirstTile, RiseLastTile, SlipperyTile);

    return Level(
        aLevelPlacing(laid, LedgeWidthTiles, LedgeHeightTiles, OnTheGround, npcs),
        theOnlyPalette(aPaletteWithSlipperyTiles()),
        loadGameData().playerData,
        npcData,
        shippedPickupData());
}

inline Level levelWithALedgeAndAWall(const std::vector<NpcSpawnData> &npcs)
{
    return levelWithALedgeAndAWall(npcs, shippedNpcData());
}
