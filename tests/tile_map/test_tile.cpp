#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "physics/aabb.hpp"
#include "tile_map/tile.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_pickup_data.hpp"

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
    TileData grippable;
    grippable.solid = grippable.grippable = true;
    REQUIRE(Tile(1, grippable).isGrippable());

    TileData solidButNotGrippable;
    solidButNotGrippable.solid = true;
    REQUIRE_FALSE(Tile(2, solidButNotGrippable).isGrippable());

    TileData deadly;
    deadly.deadly = deadly.grippable = true;
    REQUIRE_FALSE(Tile(3, deadly).isGrippable());
}

TEST_CASE("A tile cannot say two things that cancel each other out", "[Tile]")
{
    SECTION("A solid tile is never touched, so it can deliver nothing on touch")
    {
        TileData solidAndDeadly;
        solidAndDeadly.solid = solidAndDeadly.deadly = true;
        REQUIRE_THROWS(Tile(1, solidAndDeadly));

        TileData solidAndPickup;
        solidAndPickup.solid = true;
        solidAndPickup.pickup = TilePickupData{0, std::nullopt};
        REQUIRE_THROWS(Tile(1, solidAndPickup));

        TileData solidAndPortal;
        solidAndPortal.solid = solidAndPortal.portal = true;
        REQUIRE_THROWS(Tile(1, solidAndPortal));
    }

    SECTION("A deadly tile kills first, so nothing after it would happen")
    {
        TileData deadlyAndPickup;
        deadlyAndPickup.deadly = true;
        deadlyAndPickup.pickup = TilePickupData{0, std::nullopt};
        REQUIRE_THROWS(Tile(1, deadlyAndPickup));

        TileData deadlyAndPortal;
        deadlyAndPortal.deadly = deadlyAndPortal.portal = true;
        REQUIRE_THROWS(Tile(1, deadlyAndPortal));
    }

    SECTION("A pickup that is also the way out is allowed, because both happen")
    {
        TileData pickupAndPortal;
        pickupAndPortal.portal = true;
        pickupAndPortal.pickup = TilePickupData{0, std::nullopt};
        REQUIRE_NOTHROW(Tile(1, pickupAndPortal));
    }

    SECTION("Saying a tile you cannot hold on to is grippable is allowed, it just is not")
    {
        TileData grippableSpikes;
        grippableSpikes.deadly = true;
        grippableSpikes.grippable = true;
        REQUIRE_NOTHROW(Tile(1, grippableSpikes));
        REQUIRE_FALSE(Tile(1, grippableSpikes).isGrippable());
    }
}
