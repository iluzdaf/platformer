#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/physics_body.hpp"
#include "timing/fixed_time_step.hpp"
#include "physics/physics_body_data.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"
using Catch::Approx;

namespace
{
    PhysicsBody setupBody(
        glm::vec2 position = {0, 0},
        glm::vec2 velocity = {0, 0},
        glm::vec2 size = {16, 16},
        glm::vec2 offset = {0, 0},
        float stepHeight = 3.0f)
    {
        PhysicsBodyData data{size, offset, stepHeight};
        PhysicsBody body(data);
        body.setPosition(position);
        body.setVelocity(velocity);
        return body;
    }

    constexpr int Full = 1;
    constexpr int OnePixelLow = 2;
    constexpr int FivePixelsLow = 3;

    TilePaletteData surfacesOfThreeHeights()
    {
        TileData full;
        full.solid = true;
        TileData onePixelLow;
        onePixelLow.solid = true;
        onePixelLow.collider = TileColliderData{glm::vec2(0.0f, 1.0f), glm::vec2(16.0f, 15.0f)};
        TileData fivePixelsLow;
        fivePixelsLow.solid = true;
        fivePixelsLow.collider = TileColliderData{glm::vec2(0.0f, 5.0f), glm::vec2(16.0f, 11.0f)};
        return paletteOf(
            {{0, TileData{}},
             {Full, full},
             {OnePixelLow, onePixelLow},
             {FivePixelsLow, fivePixelsLow}});
    }

    TileMap aStepFrom(int lowTile)
    {
        return aTileMap(
            {{{0, 5}, lowTile}, {{1, 5}, lowTile}, {{2, 5}, Full}, {{3, 5}, Full}},
            10,
            10,
            16,
            surfacesOfThreeHeights());
    }

    void walkRight(PhysicsBody &body, const TileMap &tileMap, float pulledDownAt = 0.0f)
    {
        FixedTimeStep timeStepper;
        timeStepper.run(
            0.5f,
            [&](float deltaTime)
            {
                body.setVelocity({60.0f, pulledDownAt});
                body.stepPhysics(deltaTime, tileMap);
            });
    }
}

TEST_CASE("A body over two tiles is lifted once, not once per tile", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 5}, 1}, {{1, 5}, 1}});
    PhysicsBody body = setupBody({8, 4 * 16.0f + 1.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(4 * 16.0f));
}

TEST_CASE("A body shifted into a wall is pushed out the shortest way", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{3, 3}, 1}});
    PhysicsBody body = setupBody({3 * 16.0f - 14.0f, 3 * 16.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().x == Approx(2 * 16.0f));
    REQUIRE(body.getPosition().y == Approx(3 * 16.0f));
    REQUIRE(body.getVelocity().x == Approx(0.0f));
    REQUIRE_FALSE(body.getCollisionAABBX().isEmpty());
}

TEST_CASE("A body with its head in a ceiling is pushed down out of it", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{3, 2}, 1}});
    PhysicsBody body = setupBody({3 * 16.0f, 3 * 16.0f - 1.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(3 * 16.0f));
    REQUIRE(body.getVelocity().y == Approx(0.0f));
}

TEST_CASE("A body deeper across than down is pushed up, not sideways", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{3, 5}, 1}});
    PhysicsBody body = setupBody({3 * 16.0f + 4.0f, 4 * 16.0f + 2.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().x == Approx(3 * 16.0f + 4.0f));
    REQUIRE(body.getPosition().y == Approx(4 * 16.0f));
}

TEST_CASE(
    "A body on a lower tile beside a taller corner keeps its feet on what it feels",
    "[PhysicsBody]")
{
    TileMap tileMap = aTileMap(
        {{{0, 5}, Full}, {{1, 5}, OnePixelLow}, {{2, 5}, OnePixelLow}},
        10,
        10,
        16,
        surfacesOfThreeHeights());
    PhysicsBody body = setupBody({14, 4 * 16.0f + 1.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(4 * 16.0f + 1.0f));
    REQUIRE(body.contactWithGround(tileMap));
}

TEST_CASE(
    "A body inside a tile on the top row is pushed down, since up is out of the map",
    "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{3, 0}, 1}});
    PhysicsBody body = setupBody({3 * 16.0f, 0.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(16.0f));
    REQUIRE(body.getPosition().x == Approx(3 * 16.0f));
}

TEST_CASE("A body inside a tile on the bottom row is pushed up", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{3, 9}, 1}});
    PhysicsBody body = setupBody({3 * 16.0f, 9 * 16.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(8 * 16.0f));
}

TEST_CASE("A body inside a tile in the left column is pushed right", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 3}, 1}, {{0, 2}, 1}, {{0, 4}, 1}});
    PhysicsBody body = setupBody({2.0f, 3 * 16.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().x == Approx(16.0f));
    REQUIRE(body.getPosition().y == Approx(3 * 16.0f));
}

TEST_CASE("A body found resting inside a surface is lifted onto it", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 5}, 1}});
    PhysicsBody body = setupBody({0, 4 * 16.0f + 1.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(4 * 16.0f));
    REQUIRE(body.getVelocity().y == Approx(0.0f));
    REQUIRE_FALSE(body.getCollisionAABBY().isEmpty());
}

TEST_CASE("A body resting on a surface is left where it is", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 5}, 1}});
    PhysicsBody body = setupBody({0, 4 * 16.0f}, {0, 0});

    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().y == Approx(4 * 16.0f));
    REQUIRE(body.getCollisionAABBY().isEmpty());
}

TEST_CASE("A lip no taller than the step height is walked over", "[PhysicsBody]")
{
    TileMap tileMap = aStepFrom(OnePixelLow);
    PhysicsBody body = setupBody({8, 4 * 16.0f + 1.0f});

    walkRight(body, tileMap);

    REQUIRE(body.getPosition().x == Approx(38.0f));
    REQUIRE(body.getPosition().y == Approx(4 * 16.0f));
}

TEST_CASE("A lip taller than the step height is a wall", "[PhysicsBody]")
{
    TileMap tileMap = aStepFrom(FivePixelsLow);
    PhysicsBody body = setupBody({8, 4 * 16.0f + 5.0f});

    walkRight(body, tileMap);

    REQUIRE(body.getPosition().x == Approx(16.0f));
    REQUIRE(body.getPosition().y == Approx(4 * 16.0f + 5.0f));

    body.setVelocity({60.0f, 0.0f});
    body.stepPhysics(0.01f, tileMap);

    REQUIRE(body.getPosition().x == Approx(16.0f));
    REQUIRE_FALSE(body.getCollisionAABBX().isEmpty());
}

TEST_CASE("The step height is the body's to say", "[PhysicsBody]")
{
    TileMap tileMap = aStepFrom(FivePixelsLow);
    PhysicsBody body = setupBody({8, 4 * 16.0f + 5.0f}, {0, 0}, {16, 16}, {0, 0}, 5.0f);

    walkRight(body, tileMap);

    REQUIRE(body.getPosition().x == Approx(38.0f));
    REQUIRE(body.getPosition().y == Approx(4 * 16.0f));
}

TEST_CASE("Stepping down onto a lower surface still drops", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap(
        {{{0, 5}, Full}, {{1, 5}, Full}, {{2, 5}, OnePixelLow}, {{3, 5}, OnePixelLow}},
        10,
        10,
        16,
        surfacesOfThreeHeights());
    PhysicsBody body = setupBody({8, 4 * 16.0f});

    walkRight(body, tileMap, 20.0f);

    REQUIRE(body.getPosition().x == Approx(38.0f));
    REQUIRE(body.getPosition().y == Approx(4 * 16.0f + 1.0f));
}

TEST_CASE("A step height that leaves no body to collide with is refused", "[PhysicsBody]")
{
    REQUIRE_THROWS_WITH(
        PhysicsBody(PhysicsBodyData{{16, 16}, {0, 0}, 12.0f}),
        Catch::Matchers::ContainsSubstring("leaves nothing of the body"));
    REQUIRE_THROWS_WITH(
        PhysicsBody(PhysicsBodyData{{16, 16}, {0, 0}, -1.0f}),
        Catch::Matchers::ContainsSubstring("not a height"));
    REQUIRE_NOTHROW(PhysicsBody(PhysicsBodyData{{16, 16}, {0, 0}, 11.0f}));
}

TEST_CASE("PhysicsBody resolves collisions with solid tiles", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 5}, 1}});
    PhysicsBody body = setupBody({0, 4 * 16.0f}, {0, 980});
    FixedTimeStep timeStepper;
    timeStepper.run(1.0f, [&](float deltaTime) { body.stepPhysics(deltaTime, tileMap); });
    float expectedY = 4 * 16.0f;
    REQUIRE(body.getPosition().y == Approx(expectedY));
    REQUIRE(body.getVelocity().y == Approx(0.0f).margin(0.01f));
}

TEST_CASE("PhysicsBody clamps to map bounds", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap();
    PhysicsBody body = setupBody({-5, -5}, {0, 0});
    body.stepPhysics(1.0f, tileMap);
    REQUIRE(body.getPosition().x == Approx(0.0f));
    REQUIRE(body.getPosition().y == Approx(0.0f));
}

TEST_CASE("PhysicsBody detects contact with ground", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{1, 2}, 1}});
    PhysicsBody body = setupBody({16, 16});
    REQUIRE(body.contactWithGround(tileMap));
}

TEST_CASE("PhysicsBody detects contact with ceiling", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{1, 2}, 1}});
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithCeiling(tileMap));
}

TEST_CASE("PhysicsBody inside a tile is neither standing on it nor under it", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{1, 3}, 1}});
    PhysicsBody body = setupBody({16, 48});

    REQUIRE_FALSE(body.contactWithGround(tileMap));
    REQUIRE_FALSE(body.contactWithCeiling(tileMap));
}

TEST_CASE("PhysicsBody detects contact with left wall", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 3}, 1}});
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithLeftWall(tileMap));
}

TEST_CASE("PhysicsBody detects contact with right wall", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{2, 3}, 1}});
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithRightWall(tileMap));
}
TEST_CASE("PhysicsBody detects a wall beside its head", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 3}, 1}});
    PhysicsBody body = setupBody({16, 48});
    REQUIRE(body.contactWithLeftWallAtHead(tileMap));
}

TEST_CASE("PhysicsBody beside a ledge touches the wall but not at its head", "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{0, 4}, 1}});
    PhysicsBody body = setupBody({16, 56});
    REQUIRE(body.contactWithLeftWall(tileMap));
    REQUIRE_FALSE(body.contactWithLeftWallAtHead(tileMap));
}

TEST_CASE(
    "PhysicsBody beside a ledge on its right touches the wall but not at its head",
    "[PhysicsBody]")
{
    TileMap tileMap = aTileMap({{{2, 4}, 1}});
    PhysicsBody body = setupBody({16, 56});
    REQUIRE(body.contactWithRightWall(tileMap));
    REQUIRE_FALSE(body.contactWithRightWallAtHead(tileMap));
}

TEST_CASE("PhysicsBody knows how far its feet sit from its position", "[PhysicsBody]")
{
    PhysicsBody body = setupBody({0, 0}, {0, 0}, {6, 10}, {2, 5});

    body.setPosition(glm::vec2(100, 200) - body.getBottomCenterOffset());

    REQUIRE(body.getAABB().left() == Approx(97.0f));
    REQUIRE(body.getAABB().right() == Approx(103.0f));
    REQUIRE(body.getAABB().bottom() == Approx(200.0f));
}
