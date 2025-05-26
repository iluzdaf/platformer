#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "game/tile_map.hpp"

// TileMap getWorldHeight and width
// Tilemap initializes size correctly
// TileMap init with json, init only once

TEST_CASE("TileMap initializes grid correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.size = 1;
    tileMapData.width = 10;
    tileMapData.height = 5;
    TileMap tileMap;
    tileMap.initByData(tileMapData);

    REQUIRE(tileMap.getWidth() == 10);
    REQUIRE(tileMap.getHeight() == 5);

    for (int y = 0; y < tileMap.getHeight(); ++y)
        for (int x = 0; x < tileMap.getWidth(); ++x)
            REQUIRE(tileMap.getTileIndex(x, y) == -1);
}

TEST_CASE("TileMap set/get tile indices correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.size = 1;
    tileMapData.width = 3;
    tileMapData.height = 3;
    TileMap tileMap;
    tileMap.initByData(tileMapData);

    SECTION("Sets and gets tile indices correctly")
    {
        REQUIRE_NOTHROW(tileMap.setTileIndex(1, 1, 5));
        REQUIRE_NOTHROW(tileMap.setTileIndex(0, 2, 7));
        REQUIRE_NOTHROW(tileMap.setTileIndex(0, 0, 0));
        REQUIRE(tileMap.getTileIndex(1, 1) == 5);
        REQUIRE(tileMap.getTileIndex(0, 2) == 7);
        REQUIRE(tileMap.getTileIndex(2, 2) == -1);
        REQUIRE(tileMap.getTileIndex(0, 0) == 0);
    }

    SECTION("getTileIndex handles out of bounds by returning -1")
    {
        REQUIRE(tileMap.getTileIndex(-1, 0) == -1);
        REQUIRE(tileMap.getTileIndex(0, -1) == -1);
        REQUIRE(tileMap.getTileIndex(3, 0) == -1);
        REQUIRE(tileMap.getTileIndex(0, 3) == -1);
    }

    SECTION("setTileIndex handles out of bounds")
    {
        REQUIRE_THROWS_AS(tileMap.setTileIndex(3, 0, 1), std::out_of_range);
    }

    SECTION("setTileIndex Throws on negative value")
    {
        REQUIRE_THROWS_AS(tileMap.setTileIndex(2, 2, -5), std::invalid_argument);
    }
}

TEST_CASE("TileMap returns correct tile", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.size = 1;
    tileMapData.width = 3;
    tileMapData.height = 3;
    tileMapData.data = {{1, {TileKind::Solid}},
                        {2, {TileKind::Empty}},
                        {3, {TileKind::Empty}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);

    SECTION("Known indices")
    {
        auto tile1 = tileMap.getTile(1);
        REQUIRE(tile1.has_value());
        REQUIRE(tile1->get().getKind() == TileKind::Solid);
        REQUIRE_FALSE(tile1->get().isAnimated());

        auto tile2 = tileMap.getTile(2);
        REQUIRE(tile2.has_value());
        REQUIRE(tile2->get().getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile2->get().isAnimated());

        auto tile3 = tileMap.getTile(3);
        REQUIRE(tile3.has_value());
        REQUIRE(tile3->get().getKind() == TileKind::Empty);
        REQUIRE_FALSE(tile3->get().isAnimated());
    }

    SECTION("Unknown indices")
    {
        auto tileOpt = tileMap.getTile(999);
        REQUIRE_FALSE(tileOpt.has_value());
    }
}

TEST_CASE("TileMap animates tiles correctly", "[TileMap]")
{
    TileMapData tileMapData;
    tileMapData.size = 1;
    tileMapData.width = 2;
    tileMapData.height = 2;
    tileMapData.data = {
        {1, TileData{TileKind::Empty, TileAnimationData({10, 11, 12}, 0.1f)}},
        {2, TileData{TileKind::Empty}},
        {3, TileData{TileKind::Empty, TileAnimationData({5, 6}, 0.1f)}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    tileMap.setTileIndex(0, 0, 1);
    tileMap.setTileIndex(0, 1, 2);
    tileMap.setTileIndex(1, 1, 3);

    SECTION("Animated tiles")
    {
        auto tile = tileMap.getTile(1);
        auto tile2 = tileMap.getTile(3);
        REQUIRE(tile->get().getCurrentFrame() == 10);
        REQUIRE(tile2->get().getCurrentFrame() == 5);

        tileMap.update(0.1f);
        REQUIRE(tile->get().getCurrentFrame() == 11);
        REQUIRE(tile2->get().getCurrentFrame() == 6);
    }

    SECTION("Non-animated tiles")
    {
        auto tile = tileMap.getTile(2);
        REQUIRE(tile->get().getCurrentFrame() == -1);

        tileMap.update(1.0f);
        REQUIRE(tile->get().getCurrentFrame() == -1);
    }
}

TEST_CASE("Pickup tile is defined correctly", "[tilemap]")
{
    TileMapData tileMapData;
    tileMapData.size = 1;
    tileMapData.width = 2;
    tileMapData.height = 2;
    tileMapData.data = {{0, {TileKind::Empty}},
                        {5, {TileKind::Pickup, std::nullopt, 0}}};
    TileMap tileMap;
    tileMap.initByData(tileMapData);
    tileMap.setTileIndex(1, 1, 5);
    auto tileOpt = tileMap.getTile(5);

    REQUIRE(tileOpt.has_value());
    REQUIRE(tileOpt->get().isPickup());
    REQUIRE(tileOpt->get().getPickupReplaceIndex().value() == 0);
}