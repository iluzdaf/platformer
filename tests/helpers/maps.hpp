#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "tile_map/tile_map.hpp"

constexpr int FloorRow = 6;
constexpr int CeilingRow = 4;
constexpr int MapWidthTiles = 10;
constexpr int HighCeilingRow = 2;
constexpr int PinchColumn = 5;

inline Placed floorTiles()
{
    Placed laid;
    layRow(laid, FloorRow, 0, MapWidthTiles - 1);
    return laid;
}

inline TileMap aFloor()
{
    return aTileMap(floorTiles());
}

inline TileMap aFloorUnderOneTileOfHeadroom()
{
    Placed laid = floorTiles();
    layRow(laid, CeilingRow, 0, MapWidthTiles - 1);
    return aTileMap(laid);
}

inline TileMap aCorridorThatPinches()
{
    Placed laid = floorTiles();
    layRow(laid, HighCeilingRow, 0, MapWidthTiles - 1);
    laid.push_back({glm::ivec2(PinchColumn, FloorRow - 2), SolidTile});
    return aTileMap(laid);
}

constexpr int PlatformRow = 8;
constexpr int LeftPlatformEnd = 4;
constexpr int WideMapHeightTiles = 12;

inline Placed twoPlatforms(int gapTiles, int rowsUp = 0, int widthTiles = 20)
{
    Placed laid;
    layRow(laid, PlatformRow, 0, LeftPlatformEnd);
    layRow(laid, PlatformRow - rowsUp, LeftPlatformEnd + gapTiles + 1, widthTiles - 1);
    return laid;
}

inline TileMap twoPlatformsApart(int gapTiles, int rowsUp = 0, int widthTiles = 20)
{
    return aTileMap(twoPlatforms(gapTiles, rowsUp, widthTiles), widthTiles, WideMapHeightTiles);
}

inline glm::vec2 takeOffPosition(const TileMap &tileMap)
{
    float tileSize = static_cast<float>(tileMap.getTileSize());
    return glm::vec2(
        static_cast<float>(LeftPlatformEnd + 1) * tileSize,
        static_cast<float>(PlatformRow) * tileSize);
}

inline glm::vec2 landingPosition(const TileMap &tileMap, int gapTiles, int rowsUp = 0)
{
    float tileSize = static_cast<float>(tileMap.getTileSize());
    return glm::vec2(
        static_cast<float>(LeftPlatformEnd + gapTiles + 1) * tileSize,
        static_cast<float>(PlatformRow - rowsUp) * tileSize);
}

constexpr int FloorBelowRow = PlatformRow + 3;

inline TileMap aLedgeAboveAFloor()
{
    Placed laid;
    layRow(laid, FloorBelowRow, 0, 19);
    layRow(laid, PlatformRow, 0, LeftPlatformEnd);

    return aTileMap(laid, 20, WideMapHeightTiles);
}

inline TileMap aLedgeAboveSpikes()
{
    Placed laid;
    layRow(laid, FloorBelowRow, 0, 19, SpikeTile);
    layRow(laid, PlatformRow, 0, LeftPlatformEnd);

    return aTileMap(laid, 20, WideMapHeightTiles, 16, aPaletteWithSpikes());
}

constexpr int TallMapHeightTiles = 18;
constexpr int DeepFloorRow = 16;
constexpr int NearLedgeEnd = 2;
constexpr int FarLedgeStart = 6;
constexpr int FarLedgeEnd = 12;

inline TileMap ledgesAboveAFloor()
{
    Placed laid;
    layRow(laid, DeepFloorRow, 0, 19);
    layRow(laid, PlatformRow, 0, NearLedgeEnd);
    layRow(laid, PlatformRow, FarLedgeStart, FarLedgeEnd);

    return aTileMap(laid, 20, TallMapHeightTiles);
}

constexpr int ClimbFloorRow = 8;
constexpr int ClimbWallX = 5;
constexpr int ClimbWallTopRow = 4;

inline Placed wallFromTheFloor()
{
    Placed laid;
    layRow(laid, ClimbFloorRow, 0, 9);
    layColumn(laid, ClimbWallX, ClimbWallTopRow, ClimbFloorRow - 1);
    return laid;
}

inline TileMap aWallFromTheFloor()
{
    return aTileMap(wallFromTheFloor(), 10, 12);
}
