#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "physics/aabb.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"

TEST_CASE("A tile says what it does", "[Tile]")
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
TEST_CASE("Only a tile that does something has a shape to collide with", "[Tile]")
{
    TileData nothing;
    REQUIRE_FALSE(Tile(0, nothing).getAABBAt(glm::vec2(0.0f)).has_value());

    TileData solid;
    solid.solid = true;
    REQUIRE(Tile(1, solid).getAABBAt(glm::vec2(0.0f)).has_value());

    TileData deadly;
    deadly.deadly = true;
    REQUIRE(Tile(2, deadly).getAABBAt(glm::vec2(0.0f)).has_value());
}

TEST_CASE("A tile that does not say its shape takes a whole tile", "[Tile]")
{
    TileData solid;
    solid.solid = true;

    std::optional<AABB> aabb = Tile(1, solid).getAABBAt(glm::vec2(32.0f, 48.0f));

    REQUIRE(aabb);
    REQUIRE(aabb->position == glm::vec2(32.0f, 48.0f));
    REQUIRE(aabb->size == glm::vec2(16.0f, 16.0f));
}

TEST_CASE("Only a solid tile that says so can be gripped", "[Tile]")
{
    TileData solid;
    solid.solid = true;
    REQUIRE(Tile(1, solid).isGrippable());

    TileData ungrippable;
    ungrippable.solid = true;
    ungrippable.grippable = false;
    REQUIRE_FALSE(Tile(2, ungrippable).isGrippable());

    TileData deadly;
    deadly.deadly = true;
    REQUIRE_FALSE(Tile(3, deadly).isGrippable());
}
