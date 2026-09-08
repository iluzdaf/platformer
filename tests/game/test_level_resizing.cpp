#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "game/level_resizing.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "helpers/palettes.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_map_data.hpp"

namespace
{
    constexpr int TileSize = 16;
    constexpr Resize GrowLeft{Side::Left, true}, GrowRight{Side::Right, true},
        GrowAbove{Side::Above, true}, GrowBelow{Side::Below, true};
    constexpr Resize ShrinkLeft{Side::Left, false}, ShrinkRight{Side::Right, false},
        ShrinkAbove{Side::Above, false}, ShrinkBelow{Side::Below, false};

    using Grid = std::vector<std::vector<int>>;

    LevelData aSmallLevel()
    {
        LevelData level;
        level.tileMapData.tilePalette = "default";
        level.tileMapData.indices = {{1, 2}, {3, 4}};
        level.playerFeet = glm::vec2(8.0f, 32.0f);
        level.npcs = {NpcSpawnData{
            "someKindOfNpc",
            glm::vec2(24.0f, 32.0f),
            PatrolData{glm::vec2(8.0f, 32.0f), glm::vec2(24.0f, 32.0f)}}};
        level.pickups = {PickupSpawnData{"someKindOfPickup", glm::vec2(24.0f, 8.0f)}};
        return level;
    }

    LevelData aWiderLevel()
    {
        LevelData level;
        level.tileMapData.tilePalette = "default";
        level.tileMapData.indices = {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}};
        level.playerFeet = glm::vec2(8.0f, 32.0f);
        level.npcs = {
            NpcSpawnData{"someKindOfNpc", glm::vec2(8.0f, 32.0f), std::nullopt},
            NpcSpawnData{
                "anotherKindOfNpc",
                glm::vec2(24.0f, 32.0f),
                PatrolData{glm::vec2(24.0f, 32.0f), glm::vec2(56.0f, 32.0f)}}};
        level.pickups = {
            PickupSpawnData{"someKindOfPickup", glm::vec2(56.0f, 24.0f)},
            PickupSpawnData{"anotherKindOfPickup", glm::vec2(24.0f, 8.0f)}};
        return level;
    }
}

TEST_CASE("Growing adds an empty row or column on the side asked", "[LevelResizing]")
{
    REQUIRE(
        resizedBy(GrowRight, aSmallLevel(), TileSize).tileMapData.indices ==
        Grid{{1, 2, 0}, {3, 4, 0}});
    REQUIRE(
        resizedBy(GrowLeft, aSmallLevel(), TileSize).tileMapData.indices ==
        Grid{{0, 1, 2}, {0, 3, 4}});
    REQUIRE(
        resizedBy(GrowAbove, aSmallLevel(), TileSize).tileMapData.indices ==
        Grid{{0, 0}, {1, 2}, {3, 4}});
    REQUIRE(
        resizedBy(GrowBelow, aSmallLevel(), TileSize).tileMapData.indices ==
        Grid{{1, 2}, {3, 4}, {0, 0}});
}

TEST_CASE("Shrinking takes the row or column on the side asked", "[LevelResizing]")
{
    REQUIRE(resizedBy(ShrinkRight, aSmallLevel(), TileSize).tileMapData.indices == Grid{{1}, {3}});
    REQUIRE(resizedBy(ShrinkLeft, aSmallLevel(), TileSize).tileMapData.indices == Grid{{2}, {4}});
    REQUIRE(resizedBy(ShrinkAbove, aSmallLevel(), TileSize).tileMapData.indices == Grid{{3, 4}});
    REQUIRE(resizedBy(ShrinkBelow, aSmallLevel(), TileSize).tileMapData.indices == Grid{{1, 2}});
}

TEST_CASE("Growing to the left moves everything one tile right", "[LevelResizing]")
{
    LevelData grown = resizedBy(GrowLeft, aSmallLevel(), TileSize);

    REQUIRE(grown.playerFeet == glm::vec2(24.0f, 32.0f));
    REQUIRE(grown.npcs[0].feet == glm::vec2(40.0f, 32.0f));
    REQUIRE(grown.npcs[0].patrol->from == glm::vec2(24.0f, 32.0f));
    REQUIRE(grown.npcs[0].patrol->to == glm::vec2(40.0f, 32.0f));
    REQUIRE(grown.pickups[0].feet == glm::vec2(40.0f, 8.0f));
}

TEST_CASE("Growing above moves everything one tile down", "[LevelResizing]")
{
    LevelData grown = resizedBy(GrowAbove, aSmallLevel(), TileSize);

    REQUIRE(grown.playerFeet == glm::vec2(8.0f, 48.0f));
    REQUIRE(grown.npcs[0].feet == glm::vec2(24.0f, 48.0f));
    REQUIRE(grown.npcs[0].patrol->from == glm::vec2(8.0f, 48.0f));
    REQUIRE(grown.pickups[0].feet == glm::vec2(24.0f, 24.0f));
}

TEST_CASE("Growing to the right or below moves nothing", "[LevelResizing]")
{
    LevelData level = aSmallLevel();

    for (Resize resize : {GrowRight, GrowBelow})
    {
        LevelData grown = resizedBy(resize, level, TileSize);

        REQUIRE(grown.playerFeet == level.playerFeet);
        REQUIRE(grown.npcs == level.npcs);
        REQUIRE(grown.pickups == level.pickups);
    }
}

TEST_CASE(
    "The shift is one tile on the side that moves, and back again when it shrinks",
    "[LevelResizing]")
{
    REQUIRE(shiftOf(GrowLeft, TileSize) == glm::vec2(16.0f, 0.0f));
    REQUIRE(shiftOf(GrowAbove, TileSize) == glm::vec2(0.0f, 16.0f));
    REQUIRE(shiftOf(ShrinkLeft, TileSize) == glm::vec2(-16.0f, 0.0f));
    REQUIRE(shiftOf(ShrinkAbove, TileSize) == glm::vec2(0.0f, -16.0f));
    for (Resize resize : {GrowRight, GrowBelow, ShrinkRight, ShrinkBelow})
        REQUIRE(shiftOf(resize, TileSize) == glm::vec2(0.0f));
}

TEST_CASE("Shrinking on the left moves what is left one tile back", "[LevelResizing]")
{
    LevelData shrunk = resizedBy(ShrinkLeft, aWiderLevel(), TileSize);

    REQUIRE(shrunk.playerFeet == glm::vec2(-8.0f, 32.0f));
    REQUIRE(shrunk.npcs.size() == 1);
    REQUIRE(shrunk.npcs[0].type == "anotherKindOfNpc");
    REQUIRE(shrunk.npcs[0].feet == glm::vec2(8.0f, 32.0f));
    REQUIRE(shrunk.npcs[0].patrol->from == glm::vec2(8.0f, 32.0f));
    REQUIRE(shrunk.npcs[0].patrol->to == glm::vec2(40.0f, 32.0f));
}

TEST_CASE("Whatever stood in the column taken goes with it", "[LevelResizing]")
{
    LevelData shrunk = resizedBy(ShrinkRight, aWiderLevel(), TileSize);

    REQUIRE(shrunk.npcs.size() == 2);
    REQUIRE(shrunk.pickups.size() == 1);
    REQUIRE(shrunk.pickups[0].type == "anotherKindOfPickup");
}

TEST_CASE(
    "A patrol with an end in the column taken is dropped, and its npc kept",
    "[LevelResizing]")
{
    LevelData shrunk = resizedBy(ShrinkRight, aWiderLevel(), TileSize);

    REQUIRE(shrunk.npcs[1].type == "anotherKindOfNpc");
    REQUIRE_FALSE(shrunk.npcs[1].patrol.has_value());
    REQUIRE(shrunk.npcs[0].patrol == std::nullopt);
}

TEST_CASE("Whatever stood in the row taken goes with it", "[LevelResizing]")
{
    LevelData shrunk = resizedBy(ShrinkAbove, aWiderLevel(), TileSize);

    REQUIRE(shrunk.pickups.size() == 1);
    REQUIRE(shrunk.pickups[0].type == "someKindOfPickup");
    REQUIRE(shrunk.pickups[0].feet == glm::vec2(56.0f, 8.0f));
    REQUIRE(shrunk.npcs.size() == 2);
}

TEST_CASE("An npc standing on the bottom row stays when the top row goes", "[LevelResizing]")
{
    LevelData shrunk = resizedBy(ShrinkAbove, aWiderLevel(), TileSize);

    REQUIRE(shrunk.npcs[0].feet == glm::vec2(8.0f, 16.0f));
}

TEST_CASE(
    "Shrinking the last column leaves rows with nothing, which a tile map refuses",
    "[LevelResizing]")
{
    LevelData once = resizedBy(ShrinkLeft, aSmallLevel(), TileSize);
    LevelData twice = resizedBy(ShrinkLeft, once, TileSize);

    REQUIRE(twice.tileMapData.indices == Grid{{}, {}});
    REQUIRE_THROWS(TileMap(twice.tileMapData, theOnlyPalette(aPaletteWithASolidTile())));
}

TEST_CASE("A resized level builds a tile map of its new size", "[LevelResizing]")
{
    LevelData grown = resizedBy(GrowLeft, resizedBy(GrowBelow, aSmallLevel(), TileSize), TileSize);
    TileMap bigger(grown.tileMapData, theOnlyPalette(aPaletteWithASolidTile()));

    REQUIRE(bigger.getWidth() == 3);
    REQUIRE(bigger.getHeight() == 3);
    REQUIRE(bigger.tilePositionToTileIndex({1, 0}) == 1);

    LevelData shrunk = resizedBy(ShrinkAbove, aSmallLevel(), TileSize);
    TileMap smaller(shrunk.tileMapData, theOnlyPalette(aPaletteWithASolidTile()));

    REQUIRE(smaller.getHeight() == 1);
    REQUIRE(smaller.tilePositionToTileIndex({0, 0}) == 3);
}

TEST_CASE(
    "An npc with its feet on the bottom edge stays when a column goes elsewhere",
    "[LevelResizing]")
{
    LevelData shrunk = resizedBy(ShrinkLeft, aSmallLevel(), TileSize);

    REQUIRE(shrunk.npcs.size() == 1);
    REQUIRE(shrunk.npcs[0].feet == glm::vec2(8.0f, 32.0f));
}
