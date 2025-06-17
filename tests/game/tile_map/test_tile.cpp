#include <catch2/catch_test_macros.hpp>
#include "game/tile_map/tile.hpp"
#include "game/tile_map/tile_data.hpp"

TEST_CASE("Tile correctly stores kind", "[Tile]")
{
    Tile solidTile(1, {TileKind::Solid});
    Tile emptyTile(0, {TileKind::Empty});

    REQUIRE(solidTile.getKind() == TileKind::Solid);
    REQUIRE(emptyTile.getKind() == TileKind::Empty);
    REQUIRE(solidTile.isSolid());
    REQUIRE_FALSE(emptyTile.isSolid());
}

TEST_CASE("Tile is not animated by default", "[Tile]")
{
    Tile tile(0, {TileKind::Empty});

    REQUIRE_FALSE(tile.isAnimated());
    REQUIRE(tile.getCurrentFrame() == 0);
}

TEST_CASE("Tile becomes animated when animation is set", "[Tile]")
{
    Tile tile(0, {TileKind::Empty, TileAnimationData{{{1, 2, 3}, 0.5f}}});

    REQUIRE(tile.isAnimated());
    REQUIRE(tile.getCurrentFrame() == 1);
}

TEST_CASE("Tile updates animation over time", "[Tile]")
{
    Tile tile(0, {TileKind::Empty, TileAnimationData{{{10, 11, 12}, 0.25f}}});
    tile.update(0.25f);
    REQUIRE(tile.getCurrentFrame() == 11);

    tile.update(0.5f);
    REQUIRE(tile.getCurrentFrame() == 10);
}