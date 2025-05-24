#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "game/tile_map.hpp"

TEST_CASE("TileMap initializes grid correctly", "[tilemap]")
{
    TileMap tileMap(10, 5);

    REQUIRE(tileMap.getWidth() == 10);
    REQUIRE(tileMap.getHeight() == 5);

    for (int y = 0; y < tileMap.getHeight(); ++y)
        for (int x = 0; x < tileMap.getWidth(); ++x)
            REQUIRE(tileMap.getTileIndex(x, y) == -1);
}

TEST_CASE("TileMap sets and gets tile indices correctly", "[tilemap]")
{
    TileMap tileMap(3, 3);
    tileMap.setTileIndex(1, 1, 5);
    tileMap.setTileIndex(0, 2, 7);

    REQUIRE(tileMap.getTileIndex(1, 1) == 5);
    REQUIRE(tileMap.getTileIndex(0, 2) == 7);
    REQUIRE(tileMap.getTileIndex(2, 2) == -1);
    REQUIRE(tileMap.getTileIndex(-1, 0) == -1);
}

TEST_CASE("TileMap setTileIndex handles out of bounds", "[tilemap]")
{
    TileMap tileMap(3, 3);

    REQUIRE_THROWS_AS(tileMap.setTileIndex(3, 0, 1), std::out_of_range);
}

TEST_CASE("TileMap setTileIndex sets indices correctly", "[tilemap]")
{
    TileMap tileMap(3, 3);

    SECTION("Throws on negative value")
    {
        REQUIRE_THROWS_AS(tileMap.setTileIndex(2, 2, -5), std::invalid_argument);
    }

    SECTION("Accepts 0 and positive values")
    {
        REQUIRE_NOTHROW(tileMap.setTileIndex(0, 0, 0));
        REQUIRE_NOTHROW(tileMap.setTileIndex(0, 1, 1));
        REQUIRE_NOTHROW(tileMap.setTileIndex(1, 2, 10));
    }
}

TEST_CASE("TileMap returns correct tile for known indices", "[tilemap]")
{
    TileMap tileMap(3, 3);
    tileMap.setTiles(
        {{1, {TileKind::Solid, std::nullopt}},
         {2, {TileKind::Empty, std::nullopt}},
         {3, {TileKind::Empty, std::nullopt}}});

    auto tile1 = tileMap.getTile(1);
    auto tile2 = tileMap.getTile(2);
    auto tile3 = tileMap.getTile(3);

    REQUIRE(tile1.has_value());
    REQUIRE(tile2.has_value());
    REQUIRE(tile3.has_value());
    REQUIRE(tile1->get().getKind() == TileKind::Solid);
    REQUIRE(tile2->get().getKind() == TileKind::Empty);
    REQUIRE(tile3->get().getKind() == TileKind::Empty);
    REQUIRE_FALSE(tile1->get().isAnimated());
    REQUIRE_FALSE(tile2->get().isAnimated());
    REQUIRE_FALSE(tile3->get().isAnimated());
}

TEST_CASE("TileMap returns no tile for unknown indices", "[tilemap]")
{
    TileMap tileMap(3, 3);
    tileMap.setTiles({{1, {TileKind::Solid, std::nullopt}}});
    auto tileOpt = tileMap.getTile(999);

    REQUIRE_FALSE(tileOpt.has_value());
}

TEST_CASE("getTileIndex handles out-of-bounds safely", "[tilemap]")
{
    TileMap tileMap(3, 3);

    REQUIRE(tileMap.getTileIndex(-1, 0) == -1);
    REQUIRE(tileMap.getTileIndex(0, -1) == -1);
    REQUIRE(tileMap.getTileIndex(3, 0) == -1);
    REQUIRE(tileMap.getTileIndex(0, 3) == -1);
}

TEST_CASE("TileMap updates animated tiles", "[tilemap]")
{
    TileMap tileMap(2, 2);

    TileAnimation anim({10, 11, 12}, 0.25f);
    tileMap.setTiles({{1, {TileKind::Empty, anim}}});

    tileMap.setTileIndex(0, 0, 1);

    auto tileBefore = tileMap.getTile(1);
    REQUIRE(tileBefore->get().getCurrentFrame() == 10);

    tileMap.update(0.25f);

    auto tileAfter = tileMap.getTile(1);
    REQUIRE(tileAfter->get().getCurrentFrame() == 11);
}

TEST_CASE("TileMap update does not affect static tiles", "[tilemap]")
{
    TileMap tileMap(2, 2);
    tileMap.setTiles({{1, {TileKind::Solid, std::nullopt}}});

    tileMap.setTileIndex(0, 0, 1);
    auto tileBefore = tileMap.getTile(1);
    REQUIRE(tileBefore->get().getCurrentFrame() == -1);

    tileMap.update(1.0f);
    auto tileAfter = tileMap.getTile(1);
    REQUIRE(tileAfter->get().getCurrentFrame() == -1);
}

TEST_CASE("TileMap updates multiple animated tiles independently", "[tilemap]")
{
    TileMap tileMap(2, 2);

    TileAnimation anim1({1, 2}, 0.1f);
    TileAnimation anim2({5, 6, 7}, 0.2f);

    tileMap.setTiles({{1, {TileKind::Empty, anim1}},
                      {2, {TileKind::Solid, anim2}}});

    tileMap.update(0.2f);

    auto tile1 = tileMap.getTile(1);
    auto tile2 = tileMap.getTile(2);

    REQUIRE(tile1->get().getCurrentFrame() == 1);
    REQUIRE(tile2->get().getCurrentFrame() == 6);
}

TEST_CASE("Pickup tile is defined correctly", "[tilemap]")
{
    TileMap tileMap(3, 3, 16);
    tileMap.setTiles({{0, {TileKind::Empty}},
                      {5, {TileKind::Pickup, std::nullopt, 0}}});
    tileMap.setTileIndex(1, 1, 5);
    auto tileOpt = tileMap.getTile(5);

    REQUIRE(tileOpt.has_value());
    REQUIRE(tileOpt->get().isPickup());
    REQUIRE(tileOpt->get().getPickupReplaceIndex().value() == 0);
}