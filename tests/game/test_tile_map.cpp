#include <catch2/catch_test_macros.hpp>
#include "game/tile_map.hpp"

TEST_CASE("TileMap initializes grid correctly", "[tilemap]")
{
    TileMap tileMap(10, 5);

    REQUIRE(tileMap.getWidth() == 10);
    REQUIRE(tileMap.getHeight() == 5);

    for (int y = 0; y < tileMap.getHeight(); ++y)
        for (int x = 0; x < tileMap.getWidth(); ++x)
            REQUIRE(tileMap.getTile(x, y) == -1);
}

TEST_CASE("TileMap sets and gets tiles correctly", "[tilemap]")
{
    TileMap tileMap(3, 3);
    tileMap.setTile(1, 1, 5);
    tileMap.setTile(0, 2, 7);

    REQUIRE(tileMap.getTile(1, 1) == 5);
    REQUIRE(tileMap.getTile(0, 2) == 7);
    REQUIRE(tileMap.getTile(2, 2) == -1);
    REQUIRE(tileMap.getTile(-1, 0) == -1);
}

TEST_CASE("TileMap setTile out of bounds", "[tilemap]")
{
    TileMap tileMap(3, 3);

    REQUIRE_THROWS_AS(tileMap.setTile(3, 0, 1), std::out_of_range);
}

TEST_CASE("TileMap setTile values", "[tilemap]")
{
    TileMap tileMap(3, 3);

    SECTION("Throws on negative value")
    {
        REQUIRE_THROWS_AS(tileMap.setTile(2, 2, -5), std::invalid_argument);
    }

    SECTION("Accepts 0 and positive values")
    {
        REQUIRE_NOTHROW(tileMap.setTile(0, 0, 0));
        REQUIRE_NOTHROW(tileMap.setTile(0, 1, 1));
        REQUIRE_NOTHROW(tileMap.setTile(1, 2, 10));
    }
}

TEST_CASE("TileMap returns correct TileType for known indices", "[tilemap]")
{
    TileMap tileMap(3, 3);

    std::unordered_map<int, TileKind> types = {
        {1, TileKind::Solid},
        {2, TileKind::Empty},
        {3, TileKind::Empty}};
    tileMap.setTileTypes(types);

    REQUIRE(tileMap.getTileType(1).kind == TileKind::Solid);
    REQUIRE(tileMap.getTileType(2).kind == TileKind::Empty);
    REQUIRE(tileMap.getTileType(3).kind == TileKind::Empty);
}

TEST_CASE("TileMap returns default type for unknown tile index", "[tilemap]")
{
    TileMap tileMap(3, 3);

    std::unordered_map<int, TileKind> types = {
        {1, TileKind::Solid}};
    tileMap.setTileTypes(types);

    const TileType &tileType = tileMap.getTileType(999);
    REQUIRE(tileType.kind == TileKind::Empty);
}