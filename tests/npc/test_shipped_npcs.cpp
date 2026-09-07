#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "helpers/actors.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/shipped.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"
#include "player/player_data.hpp"

TEST_CASE("Every npc a shipped level places has somewhere to walk", "[Npc][Level]")
{
    int placed = 0;
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        Level level(
            readLevelData(entry.path().string()),
            shippedPalettes(),
            PlayerData(),
            shippedNpcData(),
            shippedPickupData());
        for (const NpcSpawnData &spawn : spawnsIn(level))
        {
            ++placed;
            INFO(
                "npc \"" << spawn.type << "\" at " << spawn.feet.x << "," << spawn.feet.y << " in "
                         << entry.path().filename().string() << " has nowhere to walk");

            Npc npc(spawnAt("villager", SpawnTile), setupNpcData());

            float startX = npc.body().position().x;
            stepNpc(npc, level, 400);

            REQUIRE(std::abs(npc.body().position().x - startX) > 1.0f);
        }
    }

    REQUIRE(placed > 0);
}

TEST_CASE("The shipped explorer walks up from the ground to a ledge and back", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("explorer", OnTheGround, OnTheGround, LedgeLeftEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("explorer"));

    const float topOfTheLedge = surfaceOf(LedgeRow);
    const float theGround = surfaceOf(GroundRow);

    bool startedOnTheFloor = false, reachedTheTop = false, cameBackDown = false;
    float previousX = npc.body().position().x;
    int standingStill = 0, longestStandingStill = 0;

    for (int step = 0; step < 4000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);

        if (!npc.observed().contacts.onGround)
            continue;

        float foot = npc.body().position().y + 16.0f;
        if (!reachedTheTop)
            startedOnTheFloor = startedOnTheFloor || foot >= theGround;
        if (startedOnTheFloor && std::abs(foot - topOfTheLedge) < 1.0f)
            reachedTheTop = true;
        if (reachedTheTop && foot >= theGround)
            cameBackDown = true;

        standingStill =
            std::abs(npc.body().position().x - previousX) < 0.01f ? standingStill + 1 : 0;
        longestStandingStill = std::max(longestStandingStill, standingStill);
        previousX = npc.body().position().x;
    }

    REQUIRE(startedOnTheFloor);
    REQUIRE(reachedTheTop);
    REQUIRE(cameBackDown);
    REQUIRE(longestStandingStill < 100);
}

TEST_CASE("The shipped villager runs from the player and settles once it is gone", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling(
        "villager",
        glm::ivec2(6, GroundRow - 1),
        glm::ivec2(2, GroundRow - 1),
        glm::ivec2(17, GroundRow - 1));
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("villager"));

    glm::vec2 crowding = footOf(npc) + glm::vec2(12.0f, 0.0f);
    float startedAt = footOf(npc).x;

    for (int step = 0; step < 300; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, crowding);
    }

    REQUIRE(footOf(npc).x < startedAt);
    REQUIRE(glm::distance(footOf(npc), crowding) > 40.0f);

    float ranTo = footOf(npc).x;
    for (int step = 0; step < 600; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
    }

    REQUIRE(footOf(npc).x != ranTo);
}

TEST_CASE("The shipped villager never freezes out in the open on its platform", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("villager", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("villager"));

    constexpr float LeftEnd = 16.0f, RightEnd = 112.0f;
    auto outInTheOpen = [](float x)
    { return std::min(std::abs(x - LeftEnd), std::abs(x - RightEnd)) > 10.0f; };

    glm::vec2 chasing = footOf(npc) + glm::vec2(8.0f, 0.0f);
    float previousX = footOf(npc).x;
    int standingStill = 0, longestOutInTheOpen = 0;

    for (int step = 0; step < 500; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, chasing);

        chasing.x = std::max(16.0f, chasing.x - 1.1f);

        standingStill = std::abs(footOf(npc).x - previousX) < 0.01f ? standingStill + 1 : 0;
        if (outInTheOpen(footOf(npc).x))
            longestOutInTheOpen = std::max(longestOutInTheOpen, standingStill);
        previousX = footOf(npc).x;
    }

    REQUIRE(longestOutInTheOpen < 100);
}

TEST_CASE(
    "The shipped villager holds its ground while the player shares its platform",
    "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("villager", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("villager"));

    glm::vec2 cornering(112.0f, 96.0f);
    for (int step = 0; step < 600; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, cornering);
    }

    float cowering = footOf(npc).x;
    REQUIRE(cowering < 32.0f);

    float wandered = cowering;
    for (int step = 0; step < 400; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, cornering);
        wandered = std::max(wandered, footOf(npc).x);
    }

    REQUIRE(wandered < 48.0f);

    for (int step = 0; step < 600; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, glm::vec2(112.0f, 192.0f));
    }

    REQUIRE(footOf(npc).x > cowering + 16.0f);
}

TEST_CASE("The shipped villager does not shuffle on the spot once it is cornered", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("villager", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("villager"));

    glm::vec2 driving(8.0f, 96.0f);
    int flips = 0;
    bool wasFacingLeft = npc.state().facingLeft;

    for (int step = 0; step < 600; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, driving);

        if (npc.state().facingLeft != wasFacingLeft)
            ++flips;
        wasFacingLeft = npc.state().facingLeft;
    }

    REQUIRE(footOf(npc).x > 96.0f);
    REQUIRE(flips < 6);
}

TEST_CASE("The shipped villager pays no mind to a player on the platform below", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("villager", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("villager"));

    float leftMost = footOf(npc).x, rightMost = footOf(npc).x;
    for (int step = 0; step < 1200; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, glm::vec2(footOf(npc).x, 128.0f));

        leftMost = std::min(leftMost, footOf(npc).x);
        rightMost = std::max(rightMost, footOf(npc).x);
    }

    REQUIRE(rightMost - leftMost > 64.0f);
}

TEST_CASE("The shipped explorer climbs the wall above the ledge", "[Npc][Level][Climb]")
{
    NpcSpawnData spawn = patrolling("explorer", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("explorer"));

    const float theLedge = surfaceOf(LedgeRow);
    const float topOfTheFace = surfaceOf(1);

    float highest = footOf(npc).y;
    int reachedTheTopAt = -1;
    bool cameBackDown = false;
    for (int step = 0; step < 4000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        highest = std::min(highest, footOf(npc).y);
        if (reachedTheTopAt < 0 && highest <= topOfTheFace + 1.0f)
            reachedTheTopAt = step;
        if (reachedTheTopAt >= 0 && footOf(npc).y >= theLedge - 1.0f)
            cameBackDown = true;
    }

    INFO("highest foot reached " << highest << " at step " << reachedTheTopAt);
    REQUIRE(highest <= topOfTheFace + 1.0f);
    REQUIRE(cameBackDown);
    REQUIRE(reachedTheTopAt < 270);
}
