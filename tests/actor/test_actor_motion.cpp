#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include "actor/actor_motion.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "physics/physics_body.hpp"
#include "physics/physics_body_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette.hpp"

namespace
{
    constexpr int Grippable = 1;
    constexpr int Slippery = 2;

    TilePalette wallsOfBothKinds()
    {
        TilePalette palette = paletteOf({{0, TileData{}}});

        TileData grippable;
        grippable.solid = true;
        grippable.grippable = true;
        palette.tiles[Grippable] = grippable;

        TileData slippery;
        slippery.solid = true;
        palette.tiles[Slippery] = slippery;

        return palette;
    }

    PhysicsBody bodyBesideWalls()
    {
        PhysicsBodyData data{glm::vec2(16.0f, 16.0f), glm::vec2(0.0f, 0.0f)};
        PhysicsBody body(data);
        body.setPosition(glm::vec2(16.0f, 48.0f));

        return body;
    }
}

TEST_CASE("ActorMotion remembers which side a grippable wall was on", "[ActorMotion]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, wallsOfBothKinds());
    tileMap.setTileIndex(glm::ivec2(0, 3), Grippable);

    ActorMotion motion{ActorMotionData()};
    motion.readContacts(bodyBesideWalls(), tileMap);

    REQUIRE(motion.getState().contacts.grippableLeftWall);
    REQUIRE(motion.getState().contacts.wasLastWallLeft);
}

TEST_CASE("ActorMotion does not remember a wall it could not grip", "[ActorMotion]")
{
    TileMap grippableOnTheLeft = setupTileMap(10, 10, 16, wallsOfBothKinds());
    grippableOnTheLeft.setTileIndex(glm::ivec2(0, 3), Grippable);

    TileMap slipperyOnTheRight = setupTileMap(10, 10, 16, wallsOfBothKinds());
    slipperyOnTheRight.setTileIndex(glm::ivec2(2, 3), Slippery);

    ActorMotion motion{ActorMotionData()};
    motion.readContacts(bodyBesideWalls(), grippableOnTheLeft);
    REQUIRE(motion.getState().contacts.wasLastWallLeft);

    motion.readContacts(bodyBesideWalls(), slipperyOnTheRight);

    REQUIRE(motion.getState().contacts.touchingRightWall);
    REQUIRE_FALSE(motion.getState().contacts.grippableRightWall);
    REQUIRE(motion.getState().contacts.wasLastWallLeft);
}
