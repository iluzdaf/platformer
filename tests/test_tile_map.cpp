#include <catch2/catch_test_macros.hpp>
#include "tile_map.hpp"

TEST_CASE("TileMap initializes grid correctly", "[tilemap]")
{
    TileMap map(10, 5);

    REQUIRE(map.getWidth() == 10);
    REQUIRE(map.getHeight() == 5);

    for (int y = 0; y < map.getHeight(); ++y)
        for (int x = 0; x < map.getWidth(); ++x)
            REQUIRE(map.getTile(x, y) == 0);
}

TEST_CASE("TileMap sets and gets tiles", "[tilemap]")
{
    TileMap map(3, 3);
    map.setTile(1, 1, 5);
    map.setTile(0, 2, 7);

    REQUIRE(map.getTile(1, 1) == 5);
    REQUIRE(map.getTile(0, 2) == 7);
    REQUIRE(map.getTile(2, 2) == 0);
}

TEST_CASE("TileMap out-of-bounds access", "[tilemap]")
{
    TileMap map(3, 3);

    REQUIRE_THROWS_AS(map.getTile(-1, 0), std::out_of_range);
    REQUIRE_THROWS_AS(map.setTile(3, 0, 1), std::out_of_range);
}