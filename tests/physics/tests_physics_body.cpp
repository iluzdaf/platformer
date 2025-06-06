

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/physics_body.hpp"
#include "physics/fixed_time_step.hpp"
#include "game/tile_map/tile_map.hpp"
#include "test_helpers/test_tilemap_utils.hpp"
using Catch::Approx;

PhysicsBody setupBody(
    glm::vec2 position = {0, 0},
    glm::vec2 size = {16, 16},
    glm::vec2 velocity = {0, 0},
    float gravity = 0.0f)
{
    PhysicsBody body;
    body.setPosition(position);
    body.setColliderSize(size);
    body.setVelocity(velocity);
    body.setGravity(gravity);
    return body;
}

TEST_CASE("PhysicsBody resolves collisions with solid tiles", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, {{1, TileData{TileKind::Solid}}});
    tileMap.setTileIndex(glm::ivec2(0, 5), 1);
    PhysicsBody body = setupBody({0, 4 * 16.0f}, {16, 16}, {0, 0}, 100.0f);
    FixedTimeStep timeStepper;
    timeStepper.run(1.0f, [&](float deltaTime)
                    { 
                        body.applyGravity(deltaTime);
                        body.stepPhysics(deltaTime, tileMap); });
    float expectedY = 4 * 16.0f;
    REQUIRE(body.getPosition().y == Approx(expectedY));
    REQUIRE(body.getVelocity().y == Approx(0.0f).margin(0.01f));
}

TEST_CASE("PhysicsBody clamps to map bounds", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap(10, 10, 16);
    PhysicsBody body = setupBody({-5, -5}, {16, 16}, {0, 0});
    body.stepPhysics(1.0f, tileMap);
    REQUIRE(body.getPosition().x == Approx(0.0f));
    REQUIRE(body.getPosition().y == Approx(0.0f));
}

TEST_CASE("PhysicsBody detects contact with ground", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap(10, 10, 16, {{1, TileData{TileKind::Solid}}});
    tileMap.setTileIndex(glm::ivec2(1, 2), 1);
    PhysicsBody body = setupBody({16, 16}, {16, 16});
    REQUIRE(body.contactWithGround(tileMap));
}

TEST_CASE("PhysicsBody applies gravity", "[PhysicsBody]")
{
    PhysicsBody body = setupBody({0, 0}, {16, 16}, {0, 0}, 9.8f);
    body.applyGravity(1.0f);
    REQUIRE(body.getVelocity().y == Approx(9.8f));
}