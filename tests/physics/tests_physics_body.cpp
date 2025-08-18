#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/physics_body.hpp"
#include "physics/fixed_time_step.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
using Catch::Approx;

PhysicsBody setupBody(
    glm::vec2 position = {0, 0},
    glm::vec2 velocity = {0, 0},
    glm::vec2 size = {16, 16},
    glm::vec2 offset = {0, 0})
{
    PhysicsBodyData data{size, offset};
    PhysicsBody body(data);
    body.setPosition(position);
    body.setVelocity(velocity);
    return body;
}

TEST_CASE("PhysicsBody resolves collisions with solid tiles", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap();
    tileMap.setTileIndex(glm::ivec2(0, 5), 1);
    PhysicsBody body = setupBody({0, 4 * 16.0f}, {0, 980});
    FixedTimeStep timeStepper;
    timeStepper.run(1.0f, [&](float deltaTime)
                    { body.stepPhysics(deltaTime, tileMap); });
    float expectedY = 4 * 16.0f;
    REQUIRE(body.getPosition().y == Approx(expectedY));
    REQUIRE(body.getVelocity().y == Approx(0.0f).margin(0.01f));
}

TEST_CASE("PhysicsBody clamps to map bounds", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap();
    PhysicsBody body = setupBody({-5, -5}, {0, 0});
    body.stepPhysics(1.0f, tileMap);
    REQUIRE(body.getPosition().x == Approx(0.0f));
    REQUIRE(body.getPosition().y == Approx(0.0f));
}

TEST_CASE("PhysicsBody detects contact with ground", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap();
    tileMap.setTileIndex(glm::ivec2(1, 2), 1);
    PhysicsBody body = setupBody({16, 16});
    REQUIRE(body.contactWithGround(tileMap));
}

TEST_CASE("PhysicsBody detects contact with ceiling", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap();
    tileMap.setTileIndex(glm::ivec2(1, 3), 1);
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithCeiling(tileMap));
}

TEST_CASE("PhysicsBody detects contact with left wall", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap();
    tileMap.setTileIndex(glm::ivec2(0, 3), 1);
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithLeftWall(tileMap));
}

TEST_CASE("PhysicsBody detects contact with right wall", "[PhysicsBody]")
{
    TileMap tileMap = setupTileMap();
    tileMap.setTileIndex(glm::ivec2(2, 3), 1);
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithRightWall(tileMap));
}