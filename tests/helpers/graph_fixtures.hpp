#pragma once

#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"

constexpr int FloorRow = 6;
constexpr int CeilingRow = 4;
constexpr int MapWidthTiles = 10;
constexpr int HighCeilingRow = 2;
constexpr int PinchColumn = 5;

inline NavigationProfile profileOfHeight(float height)
{
    NavigationProfile profile;
    profile.physicsBodyData.colliderSize = glm::vec2(8.0f, height);
    return profile;
}

inline NavigationProfile standardProfile()
{
    return profileOfHeight(13.0f);
}

using Placed = std::vector<std::pair<glm::ivec2, int>>;

inline void layRow(Placed &laid, int row, int fromX, int toX)
{
    for (int x = fromX; x <= toX; ++x)
        laid.push_back({glm::ivec2(x, row), 1});
}

inline void layFloor(Placed &laid, int groundY, int fromX, int toX)
{
    layRow(laid, groundY, fromX, toX);
}

inline Placed floorTiles()
{
    Placed laid;
    layRow(laid, FloorRow, 0, MapWidthTiles - 1);
    return laid;
}

inline TileMap setupFloor()
{
    return aTileMapWith(floorTiles());
}

inline TileMap setupFloorUnderOneTileOfHeadroom()
{
    Placed laid = floorTiles();
    layRow(laid, CeilingRow, 0, MapWidthTiles - 1);
    return aTileMapWith(laid);
}

inline TileMap setupCorridorThatPinches()
{
    Placed laid = floorTiles();
    layRow(laid, HighCeilingRow, 0, MapWidthTiles - 1);
    laid.push_back({glm::ivec2(PinchColumn, FloorRow - 2), 1});
    return aTileMapWith(laid);
}

constexpr int PlatformRow = 8;
constexpr int LeftPlatformEnd = 4;
constexpr int WideMapHeightTiles = 12;

inline ActorMotionData jumperMotionData()
{
    ActorMotionData motionData;
    motionData.moveAbilityData = MoveAbilityData{};
    motionData.gravityAbilityData = GravityAbilityData{};
    motionData.jumpAbilityData = JumpAbilityData{};
    return motionData;
}

inline ActorMotionData fallerMotionData()
{
    ActorMotionData motionData;
    motionData.gravityAbilityData = GravityAbilityData{};
    return motionData;
}

inline NavigationProfile profileThatMoves(float height, const ActorMotionData &motionData)
{
    NavigationProfile profile = profileOfHeight(height);
    profile.jumpArcs = simulateJumpArcs(motionData);
    profile.motionData = motionData;
    return profile;
}

inline NavigationProfile jumperProfile()
{
    return profileThatMoves(13.0f, jumperMotionData());
}

inline NavigationProfile climberProfile()
{
    NavigationProfile profile = profileOfHeight(13.0f);
    ActorMotionData motionData;
    motionData.wallHangAbilityData = WallHangAbilityData();
    motionData.wallClimbAbilityData = WallClimbAbilityData();
    profile.motionData = motionData;
    return profile;
}

inline Placed twoPlatforms(int gapTiles, int rowsUp = 0, int widthTiles = 20)
{
    Placed laid;
    layFloor(laid, PlatformRow, 0, LeftPlatformEnd);
    layFloor(laid, PlatformRow - rowsUp, LeftPlatformEnd + gapTiles + 1, widthTiles - 1);
    return laid;
}

inline TileMap setupTwoPlatforms(int gapTiles, int rowsUp = 0, int widthTiles = 20)
{
    return aTileMapWith(twoPlatforms(gapTiles, rowsUp, widthTiles), widthTiles, WideMapHeightTiles);
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

inline TileMap setupLedgeAboveFloor()
{
    Placed laid;
    layFloor(laid, FloorBelowRow, 0, 19);
    layFloor(laid, PlatformRow, 0, LeftPlatformEnd);
    TileMap tileMap = aTileMapWith(laid, 20, WideMapHeightTiles);

    return tileMap;
}

constexpr int SpikeTileIndex = 2;

inline TilePaletteData paletteWithSpikes()
{
    TilePaletteData palette = aPaletteWithASolidTile();
    TileData spikes;
    spikes.deadly = true;
    palette.tiles[SpikeTileIndex] = spikes;
    return palette;
}

inline TileMap setupLedgeAboveSpikes()
{
    Placed laid;
    for (int x = 0; x < 20; ++x)
        laid.push_back({glm::ivec2(x, FloorBelowRow), SpikeTileIndex});
    layFloor(laid, PlatformRow, 0, LeftPlatformEnd);

    return aTileMapWith(laid, 20, WideMapHeightTiles, 16, paletteWithSpikes());
}

constexpr int TallMapHeightTiles = 18;
constexpr int DeepFloorRow = 16;
constexpr int NearLedgeEnd = 2;
constexpr int FarLedgeStart = 6;
constexpr int FarLedgeEnd = 12;

inline TileMap setupLedgesAboveFloor()
{
    Placed laid;
    layFloor(laid, DeepFloorRow, 0, 19);
    layFloor(laid, PlatformRow, 0, NearLedgeEnd);
    layFloor(laid, PlatformRow, FarLedgeStart, FarLedgeEnd);

    return aTileMapWith(laid, 20, TallMapHeightTiles);
}

constexpr int ClimbFloorRow = 8;
constexpr int ClimbWallX = 5;
constexpr int ClimbWallTopRow = 4;

inline Placed wallFromTheFloor()
{
    Placed laid;
    layFloor(laid, ClimbFloorRow, 0, 9);
    for (int y = ClimbWallTopRow; y < ClimbFloorRow; ++y)
        laid.push_back({glm::ivec2(ClimbWallX, y), 1});
    return laid;
}

inline TileMap setupWallFromTheFloor()
{
    return aTileMapWith(wallFromTheFloor(), 10, 12);
}
