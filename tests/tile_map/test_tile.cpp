#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "physics/aabb.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_collider_data.hpp"

namespace
{
    constexpr glm::vec2 Cell{16.0f, 16.0f};
}

TEST_CASE("A tile says what it does", "[Tile]")
{
    TileData solidTileData, emptyTileData;
    solidTileData.solid = true;
    Tile solidTile(solidTileData, Cell);
    Tile emptyTile(emptyTileData, Cell);

    REQUIRE(solidTile.isSolid());
    REQUIRE(emptyTile.isEmpty());
    REQUIRE(solidTile.isSolid());
    REQUIRE_FALSE(emptyTile.isSolid());
}

TEST_CASE("Tile is not animated by default", "[Tile]")
{
    TileData emptyTileData;
    Tile tile(emptyTileData, Cell);

    REQUIRE_FALSE(tile.animatingTo().has_value());
}

TEST_CASE("Tile becomes animated when animation is set", "[Tile]")
{
    TileData tileData;
    tileData.animationData = {{{1, 2, 3}, 0.5f}};
    Tile tile(tileData, Cell);

    REQUIRE(tile.animatingTo().value_or(0) == 1);
}

TEST_CASE("Tile updates animation over time", "[Tile]")
{
    TileData tileData;
    tileData.animationData = {{{10, 11, 12}, 0.25f}};
    Tile tile(tileData, Cell);
    tile.update(0.25f);
    REQUIRE(tile.animatingTo().value_or(0) == 11);

    tile.update(0.5f);
    REQUIRE(tile.animatingTo().value_or(0) == 10);
}
TEST_CASE("Only a tile that does something has a shape to collide with", "[Tile]")
{
    TileData nothing;
    REQUIRE_FALSE(Tile(nothing, Cell).getAABBAt(glm::vec2(0.0f)).has_value());

    TileData solid;
    solid.solid = true;
    REQUIRE(Tile(solid, Cell).getAABBAt(glm::vec2(0.0f)).has_value());

    TileData deadly;
    deadly.deadly = true;
    REQUIRE(Tile(deadly, Cell).getAABBAt(glm::vec2(0.0f)).has_value());
}

TEST_CASE("A tile that does not say its shape takes a whole tile", "[Tile]")
{
    TileData solid;
    solid.solid = true;

    std::optional<AABB> aabb = Tile(solid, Cell).getAABBAt(glm::vec2(32.0f, 48.0f));

    REQUIRE(aabb);
    REQUIRE(aabb->position == glm::vec2(32.0f, 48.0f));
    REQUIRE(aabb->size == glm::vec2(16.0f, 16.0f));
}

TEST_CASE("Only a solid tile that says so can be gripped", "[Tile]")
{
    TileData grippable;
    grippable.solid = grippable.grippable = true;
    REQUIRE(Tile(grippable, Cell).isGrippable());

    TileData solidButNotGrippable;
    solidButNotGrippable.solid = true;
    REQUIRE_FALSE(Tile(solidButNotGrippable, Cell).isGrippable());

    TileData deadly;
    deadly.deadly = deadly.grippable = true;
    REQUIRE_FALSE(Tile(deadly, Cell).isGrippable());
}

TEST_CASE("A tile cannot say two things that cancel each other out", "[Tile]")
{
    SECTION("A solid tile is never touched, so it can deliver nothing on touch")
    {
        TileData solidAndDeadly;
        solidAndDeadly.solid = solidAndDeadly.deadly = true;
        REQUIRE_THROWS(Tile(solidAndDeadly, Cell));

        TileData solidAndPortal;
        solidAndPortal.solid = solidAndPortal.portal = true;
        REQUIRE_THROWS(Tile(solidAndPortal, Cell));
    }

    SECTION("A deadly tile kills first, so nothing after it would happen")
    {
        TileData deadlyAndPortal;
        deadlyAndPortal.deadly = deadlyAndPortal.portal = true;
        REQUIRE_THROWS(Tile(deadlyAndPortal, Cell));
    }

    SECTION("Saying a tile you cannot hold on to is grippable is allowed, it just is not")
    {
        TileData grippableSpikes;
        grippableSpikes.deadly = true;
        grippableSpikes.grippable = true;
        REQUIRE_NOTHROW(Tile(grippableSpikes, Cell));
        REQUIRE_FALSE(Tile(grippableSpikes, Cell).isGrippable());
    }
}

TEST_CASE("A tile said nothing about fills whatever cell it is drawn in", "[Tile]")
{
    Tile small(TileData{}, glm::vec2(8.0f, 8.0f));
    Tile large(TileData{}, glm::vec2(32.0f, 24.0f));

    TileData solidData;
    solidData.solid = true;

    REQUIRE(Tile(solidData, glm::vec2(8.0f)).getAABBAt(glm::vec2(0.0f))->size == glm::vec2(8.0f));
    REQUIRE(
        Tile(solidData, glm::vec2(32.0f, 24.0f)).getAABBAt(glm::vec2(0.0f))->size ==
        glm::vec2(32.0f, 24.0f));
}

TEST_CASE("A collider a tile does name is the one it gets", "[Tile]")
{
    TileData said;
    said.solid = true;
    said.collider = TileColliderData{glm::vec2(2.0f, 3.0f), glm::vec2(4.0f, 5.0f)};

    Tile tile(said, glm::vec2(32.0f));

    REQUIRE(tile.getAABBAt(glm::vec2(10.0f))->position == glm::vec2(12.0f, 13.0f));
    REQUIRE(tile.getAABBAt(glm::vec2(10.0f))->size == glm::vec2(4.0f, 5.0f));
}

TEST_CASE("A collider of no size is refused", "[Tile]")
{
    TileData said;
    said.solid = true;
    said.collider = TileColliderData{glm::vec2(0.0f), glm::vec2(0.0f)};

    REQUIRE_THROWS_WITH(Tile(said, Cell), Catch::Matchers::ContainsSubstring("no size"));
}

TEST_CASE("A collider with no width or no height is refused too", "[Tile]")
{
    TileData wide;
    wide.solid = true;
    wide.collider = TileColliderData{glm::vec2(0.0f), glm::vec2(16.0f, 0.0f)};

    TileData tall;
    tall.solid = true;
    tall.collider = TileColliderData{glm::vec2(0.0f), glm::vec2(0.0f, 16.0f)};

    REQUIRE_THROWS(Tile(wide, Cell));
    REQUIRE_THROWS(Tile(tall, Cell));
}

TEST_CASE("A collider facing backwards is refused", "[Tile]")
{
    TileData said;
    said.solid = true;
    said.collider = TileColliderData{glm::vec2(0.0f), glm::vec2(-16.0f, 16.0f)};

    REQUIRE_THROWS(Tile(said, Cell));
}

TEST_CASE("A collider reaching past its cell is refused", "[Tile]")
{
    TileData wide;
    wide.solid = true;
    wide.collider = TileColliderData{glm::vec2(0.0f), glm::vec2(32.0f, 16.0f)};

    REQUIRE_THROWS_WITH(Tile(wide, Cell), Catch::Matchers::ContainsSubstring("outside its cell"));
}

TEST_CASE("A collider starting before its cell is refused", "[Tile]")
{
    TileData shifted;
    shifted.solid = true;
    shifted.collider = TileColliderData{glm::vec2(-1.0f, 0.0f), glm::vec2(16.0f)};

    REQUIRE_THROWS(Tile(shifted, Cell));
}

TEST_CASE("A collider filling its cell exactly is allowed", "[Tile]")
{
    TileData exact;
    exact.solid = true;
    exact.collider = TileColliderData{glm::vec2(0.0f), Cell};

    REQUIRE_NOTHROW(Tile(exact, Cell));
}

TEST_CASE("A collider is measured against the cell it is drawn in", "[Tile]")
{
    TileData sixteen;
    sixteen.solid = true;
    sixteen.collider = TileColliderData{glm::vec2(0.0f), glm::vec2(16.0f)};

    REQUIRE_NOTHROW(Tile(sixteen, glm::vec2(16.0f)));
    REQUIRE_THROWS(Tile(sixteen, glm::vec2(8.0f)));
}
