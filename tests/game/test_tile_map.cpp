#include <catch2/catch_test_macros.hpp>
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
    tileMap.setTiles({{1, TileKind::Solid},
                      {2, TileKind::Empty},
                      {3, TileKind::Empty}});

    auto tile1 = tileMap.getTile(1);
    auto tile2 = tileMap.getTile(2);
    auto tile3 = tileMap.getTile(3);

    REQUIRE(tile1.has_value());
    REQUIRE(tile2.has_value());
    REQUIRE(tile3.has_value());

    REQUIRE(tile1->get().getKind() == TileKind::Solid);
    REQUIRE(tile2->get().getKind() == TileKind::Empty);
    REQUIRE(tile3->get().getKind() == TileKind::Empty);
}

TEST_CASE("TileMap returns no tile for unknown indices", "[tilemap]")
{
    TileMap tileMap(3, 3);
    tileMap.setTiles({{1, TileKind::Solid}});
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