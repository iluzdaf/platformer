#include <catch2/catch_test_macros.hpp>
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"

TEST_CASE("Tile correctly stores kind", "[Tile]")
{
    TileData solidTileData, emptyTileData;
    solidTileData.solid = true;
    Tile solidTile(1, solidTileData);
    Tile emptyTile(0, emptyTileData);

    REQUIRE(solidTile.isSolid());
    REQUIRE(emptyTile.isEmpty());
    REQUIRE(solidTile.isSolid());
    REQUIRE_FALSE(emptyTile.isSolid());
}

TEST_CASE("Tile is not animated by default", "[Tile]")
{
    TileData emptyTileData;
    Tile tile(0, emptyTileData);

    REQUIRE_FALSE(tile.isAnimated());
    REQUIRE(tile.getCurrentFrame() == 0);
}

TEST_CASE("Tile becomes animated when animation is set", "[Tile]")
{
    TileData tileData;
    tileData.animationData = {{{1, 2, 3}, 0.5f}};
    Tile tile(0, tileData);

    REQUIRE(tile.isAnimated());
    REQUIRE(tile.getCurrentFrame() == 1);
}

TEST_CASE("Tile updates animation over time", "[Tile]")
{
    TileData tileData;
    tileData.animationData = {{{10, 11, 12}, 0.25f}};
    Tile tile(0, tileData);
    tile.update(0.25f);
    REQUIRE(tile.getCurrentFrame() == 11);

    tile.update(0.5f);
    REQUIRE(tile.getCurrentFrame() == 10);
}