#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include "actor/observing.hpp"
#include "actor/actor_contact_state.hpp"
#include "physics/physics_body.hpp"
#include "physics/physics_body_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    PhysicsBody bodyBesideWalls()
    {
        PhysicsBodyData data{glm::vec2(16.0f, 16.0f), glm::vec2(0.0f, 0.0f)};
        PhysicsBody body(data);
        body.setPosition(glm::vec2(16.0f, 48.0f));

        return body;
    }
}

TEST_CASE("Contacts remember which side a grippable wall was on", "[Observing]")
{
    TileMap tileMap = aTileMap({{{0, 3}, SolidTile}}, 10, 10, 16, aPaletteWithSlipperyTiles());

    ActorContactState contacts = contactsAfterStep(ActorContactState{}, bodyBesideWalls(), tileMap);

    REQUIRE(contacts.grippableLeftWall);
    REQUIRE(contacts.wasLastWallLeft);
}

TEST_CASE("Contacts do not remember a wall that could not be gripped", "[Observing]")
{
    TileMap grippableOnTheLeft =
        aTileMap({{{0, 3}, SolidTile}}, 10, 10, 16, aPaletteWithSlipperyTiles());

    TileMap slipperyOnTheRight =
        aTileMap({{{2, 3}, SlipperyTile}}, 10, 10, 16, aPaletteWithSlipperyTiles());

    ActorContactState contacts =
        contactsAfterStep(ActorContactState{}, bodyBesideWalls(), grippableOnTheLeft);
    REQUIRE(contacts.wasLastWallLeft);

    contacts = contactsAfterStep(contacts, bodyBesideWalls(), slipperyOnTheRight);

    REQUIRE(contacts.touchingRightWall);
    REQUIRE_FALSE(contacts.grippableRightWall);
    REQUIRE(contacts.wasLastWallLeft);
}
