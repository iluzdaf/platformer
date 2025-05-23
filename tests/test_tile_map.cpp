#include <catch2/catch_test_macros.hpp>
#include "game/tile_map.hpp"

TEST_CASE("TileMap initializes grid correctly", "[tilemap]")
{
    TileMap map(10, 5);

    REQUIRE(map.getWidth() == 10);
    REQUIRE(map.getHeight() == 5);

    for (int y = 0; y < map.getHeight(); ++y)
        for (int x = 0; x < map.getWidth(); ++x)
            REQUIRE(map.getTile(x, y) == -1);
}

TEST_CASE("TileMap sets and gets tiles correctly", "[tilemap]")
{
    TileMap map(3, 3);
    map.setTile(1, 1, 5);
    map.setTile(0, 2, 7);

    REQUIRE(map.getTile(1, 1) == 5);
    REQUIRE(map.getTile(0, 2) == 7);
    REQUIRE(map.getTile(2, 2) == -1);
    REQUIRE(map.getTile(-1, 0) == -1);
}

TEST_CASE("TileMap setTile out of bounds", "[tilemap]")
{
    TileMap map(3, 3);

    REQUIRE_THROWS_AS(map.setTile(3, 0, 1), std::out_of_range);
}

TEST_CASE("TileMap setTile values", "[tilemap]")
{
    TileMap map(3, 3);

    SECTION("Throws on negative value")
    {
        REQUIRE_THROWS_AS(map.setTile(2, 2, -5), std::invalid_argument);
    }

    SECTION("Accepts 0 and positive values")
    {
        REQUIRE_NOTHROW(map.setTile(0, 0, 0));
        REQUIRE_NOTHROW(map.setTile(0, 1, 1));
        REQUIRE_NOTHROW(map.setTile(1, 2, 10));
    }
}

TEST_CASE("TileMap returns correct TileType for known indices", "[tilemap]")
{
    TileMap map(3, 3);

    std::unordered_map<int, TileKind> types = {
        {1, TileKind::Solid},
        {2, TileKind::Empty},
        {3, TileKind::Empty}};
    map.setTileTypes(types);

    REQUIRE(map.getTileType(1).kind == TileKind::Solid);
    REQUIRE(map.getTileType(2).kind == TileKind::Empty);
    REQUIRE(map.getTileType(3).kind == TileKind::Empty);
}

TEST_CASE("TileMap returns default type for unknown tile index", "[tilemap]")
{
    TileMap map(3, 3);

    std::unordered_map<int, TileKind> types = {
        {1, TileKind::Solid}};
    map.setTileTypes(types);

    const TileType &type = map.getTileType(999);
    REQUIRE(type.kind == TileKind::Empty);
}