#include <catch2/catch_test_macros.hpp>
#include "game/tile_map/tile_map.hpp"
#include "game/tile_map/tile_map_data.hpp"

TEST_CASE("TileMap initializes grid correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.width = 10;
    tileMapData.height = 5;
    TileMap tileMap(tileMapData);

    REQUIRE(tileMap.getWidth() == 10);
    REQUIRE(tileMap.getHeight() == 5);
    for (int tileY = 0; tileY < tileMap.getHeight(); ++tileY)
        for (int tileX = 0; tileX < tileMap.getWidth(); ++tileX)
            REQUIRE(tileMap.getTileIndex(glm::ivec2(tileX, tileY)) == -1);
    REQUIRE(tileMap.getTileSize() == 16);
}

TEST_CASE("TileMap set/get tile indices correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.width = 3;
    tileMapData.height = 3;
    TileMap tileMap(tileMapData);

    SECTION("Sets and gets tile indices correctly")
    {
        REQUIRE_NOTHROW(tileMap.setTileIndex(glm::ivec2(1, 1), 5));
        REQUIRE_NOTHROW(tileMap.setTileIndex(glm::ivec2(0, 2), 7));
        REQUIRE_NOTHROW(tileMap.setTileIndex(glm::ivec2(0, 0), 0));
        REQUIRE(tileMap.getTileIndex(glm::ivec2(1, 1)) == 5);
        REQUIRE(tileMap.getTileIndex(glm::ivec2(0, 2)) == 7);
        REQUIRE(tileMap.getTileIndex(glm::ivec2(2, 2)) == -1);
        REQUIRE(tileMap.getTileIndex(glm::ivec2(0, 0)) == 0);
    }

    SECTION("getTileIndex handles out of bounds by returning -1")
    {
        REQUIRE(tileMap.getTileIndex(glm::ivec2(-1, 0)) == -1);
        REQUIRE(tileMap.getTileIndex(glm::ivec2(0, -1)) == -1);
        REQUIRE(tileMap.getTileIndex(glm::ivec2(3, 0)) == -1);
        REQUIRE(tileMap.getTileIndex(glm::ivec2(0, 3)) == -1);
    }

    SECTION("setTileIndex handles out of bounds")
    {
        REQUIRE_THROWS_AS(tileMap.setTileIndex(glm::ivec2(3, 0), 1), std::out_of_range);
    }

    SECTION("setTileIndex Throws on negative value")
    {
        REQUIRE_THROWS_AS(tileMap.setTileIndex(glm::ivec2(2, 2), -5), std::invalid_argument);
    }
}

TEST_CASE("TileMap returns correct tile", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.width = 3;
    tileMapData.height = 3;
    tileMapData.tileData = {{1, {TileKind::Solid}},
                            {2, {TileKind::Empty}},
                            {3, {TileKind::Empty}}};
    TileMap tileMap(tileMapData);

    SECTION("Known indices")
    {
        const Tile &tile1 = tileMap.getTile(1);
        REQUIRE(tile1.getKind() == TileKind::Solid);
        REQUIRE_FALSE(tile1.isAnimated());

        const Tile &tile2 = tileMap.getTile(2);
        REQUIRE(tile2.getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile2.isAnimated());

        const Tile &tile3 = tileMap.getTile(3);
        REQUIRE(tile3.getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile3.isAnimated());
    }

    SECTION("Unknown indices")
    {
        const Tile &tile = tileMap.getTile(999);
        REQUIRE(tile.getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile.isAnimated());
    }
}

TEST_CASE("TileMap animates tiles correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.width = 2;
    tileMapData.height = 2;
    tileMapData.tileData = {
        {1, {TileKind::Empty, TileAnimationData{{{10, 11, 12}, 0.1f}}}},
        {2, {TileKind::Empty}},
        {3, {TileKind::Empty, TileAnimationData{{{5, 6}, 0.1f}}}}};
    TileMap tileMap(tileMapData);
    tileMap.setTileIndex(glm::ivec2(0, 0), 1);
    tileMap.setTileIndex(glm::ivec2(0, 1), 2);
    tileMap.setTileIndex(glm::ivec2(1, 1), 3);

    SECTION("Animated tiles")
    {
        const Tile &tile1 = tileMap.getTile(1);
        const Tile &tile2 = tileMap.getTile(3);
        REQUIRE(tile1.getCurrentFrame() == 10);
        REQUIRE(tile2.getCurrentFrame() == 5);
        tileMap.update(0.1f);
        REQUIRE(tile1.getCurrentFrame() == 11);
        REQUIRE(tile2.getCurrentFrame() == 6);
    }

    SECTION("Non-animated tiles")
    {
        const Tile &tile = tileMap.getTile(2);
        REQUIRE(tile.getCurrentFrame() == -1);
        tileMap.update(1.0f);
        REQUIRE(tile.getCurrentFrame() == -1);
    }
}

TEST_CASE("Pickup tile is defined correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.width = 2;
    tileMapData.height = 2;
    tileMapData.tileData = {{0, {TileKind::Empty}},
                            {5, {TileKind::Pickup, std::nullopt, 0}}};
    TileMap tileMap(tileMapData);
    tileMap.setTileIndex(glm::ivec2(1, 1), 5);
    const Tile &tile = tileMap.getTile(5);
    REQUIRE(tile.isPickup());
    REQUIRE(tile.getPickupReplaceIndex().value() == 0);
}

TEST_CASE("TileMap calculates world dimensions correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.width = 5;
    tileMapData.height = 4;
    tileMapData.size = 16;
    TileMap tileMap(tileMapData);
    REQUIRE(tileMap.getWorldWidth() == 5 * 16);
    REQUIRE(tileMap.getWorldHeight() == 4 * 16);
}