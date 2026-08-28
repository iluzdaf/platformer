#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include "navigation/jump_simulation.hpp"
#include "actor/actor_motion_data.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "physics/physics_body_data.hpp"
#include "tile_map/tile_map.hpp"
#include <vector>
#include <cmath>

namespace
{
    ActorMotionData walkerMotionData()
    {
        ActorMotionData motionData;
        motionData.moveAbilityData = MoveAbilityData{};
        motionData.gravityAbilityData = GravityAbilityData{};
        return motionData;
    }

    ActorMotionData jumperMotionData()
    {
        ActorMotionData motionData = walkerMotionData();
        motionData.jumpAbilityData = JumpAbilityData{};
        return motionData;
    }

    float peakHeightOf(const std::vector<glm::vec2> &arc)
    {
        float highest = 0.0f;
        for (const glm::vec2 &offset : arc)
            highest = std::min(highest, offset.y);
        return -highest;
    }

    float reachOf(const std::vector<glm::vec2> &arc)
    {
        float furthest = 0.0f;
        for (const glm::vec2 &offset : arc)
            furthest = std::max(furthest, std::abs(offset.x));
        return furthest;
    }
}

TEST_CASE("An actor without a jump ability has no arc", "[JumpArc]")
{
    REQUIRE(simulateJumpArc(walkerMotionData()).offsets.empty());
}

TEST_CASE("An actor without gravity never comes back down", "[JumpArc]")
{
    ActorMotionData motionData = jumperMotionData();
    motionData.gravityAbilityData.reset();

    REQUIRE(simulateJumpArc(motionData).offsets.empty());
}

TEST_CASE("An arc leaves from where the actor stands", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperMotionData()).offsets;

    REQUIRE_FALSE(arc.empty());
    REQUIRE(arc.front() == glm::vec2(0.0f));
}

TEST_CASE("An arc rises and then returns to the height it left", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperMotionData()).offsets;

    REQUIRE(peakHeightOf(arc) > 0.0f);
    REQUIRE(arc.back().y >= 0.0f);
}

TEST_CASE("An arc only ever moves further from where it left", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperMotionData()).offsets;

    for (size_t index = 1; index < arc.size(); ++index)
        REQUIRE(arc[index].x >= arc[index - 1].x);
}

TEST_CASE("The default jump clears three tiles and crosses eight", "[JumpArc]")
{
    std::vector<glm::vec2> arc = simulateJumpArc(jumperMotionData()).offsets;
    constexpr float TileSize = 16.0f;

    REQUIRE(peakHeightOf(arc) / TileSize > 3.0f);
    REQUIRE(peakHeightOf(arc) / TileSize < 4.0f);
    REQUIRE(reachOf(arc) / TileSize > 8.0f);
    REQUIRE(reachOf(arc) / TileSize < 9.0f);
}

TEST_CASE("A stronger jump reaches higher than a weaker one", "[JumpArc]")
{
    ActorMotionData weak = jumperMotionData();
    weak.jumpAbilityData->jumpSpeed = -150.0f;

    REQUIRE(
        peakHeightOf(simulateJumpArc(weak).offsets) <
        peakHeightOf(simulateJumpArc(jumperMotionData()).offsets));
}

TEST_CASE("A jump held longer reaches further than one cut short", "[JumpArc]")
{
    ActorMotionData brief = jumperMotionData();
    brief.jumpAbilityData->jumpDuration = 0.1f;

    REQUIRE(
        reachOf(simulateJumpArc(brief).offsets) <
        reachOf(simulateJumpArc(jumperMotionData()).offsets));
}

TEST_CASE("An actor that cannot move jumps straight up", "[JumpArc]")
{
    ActorMotionData motionData = jumperMotionData();
    motionData.moveAbilityData.reset();

    std::vector<glm::vec2> arc = simulateJumpArc(motionData).offsets;

    REQUIRE_FALSE(arc.empty());
    REQUIRE(peakHeightOf(arc) > 0.0f);
    REQUIRE(reachOf(arc) == 0.0f);
}

TEST_CASE("An arc knows how long the jump was held for", "[JumpArc]")
{
    JumpArc arc = simulateJumpArc(jumperMotionData());

    REQUIRE(arc.holdDuration == jumperMotionData().jumpAbilityData->jumpDuration);
}

TEST_CASE("A shorter hold is recorded as one", "[JumpArc]")
{
    JumpArc arc = simulateJumpArc(jumperMotionData(), 0.5f);

    REQUIRE(arc.holdDuration == jumperMotionData().jumpAbilityData->jumpDuration * 0.5f);
    REQUIRE(reachOf(arc.offsets) < reachOf(simulateJumpArc(jumperMotionData()).offsets));
}

TEST_CASE("Every arc offered carries its own hold", "[JumpArc]")
{
    std::vector<JumpArc> arcs = simulateJumpArcs(jumperMotionData());

    REQUIRE(arcs.size() > 1);
    for (size_t index = 1; index < arcs.size(); ++index)
    {
        REQUIRE(arcs[index].holdDuration < arcs[index - 1].holdDuration);
        REQUIRE(reachOf(arcs[index].offsets) < reachOf(arcs[index - 1].offsets));
    }
}

TEST_CASE("A jump comes to rest on the surface, not beside it", "[JumpArc]")
{
    constexpr int TileSize = 16;
    std::vector<std::vector<int>> rows(12, std::vector<int>(30, 0));
    for (int x = 0; x < 30; ++x)
        rows[9][x] = 1;
    for (int x = 16; x < 30; ++x)
        rows[7][x] = 1;

    TileMapData tileMapData;
    tileMapData.size = TileSize;
    tileMapData.indices = rows;
    tileMapData.tilePalette = "default";
    TileMap tileMap(tileMapData, palettesFrom(getDefaultTileDataMap()));

    PhysicsBodyData physicsBodyData;
    physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
    physicsBodyData.colliderOffset = glm::vec2(4.0f, 3.0f);

    bool landedSomewhere = false;
    for (float takeOffX = 200.0f; takeOffX <= 250.0f; takeOffX += 1.0f)
    {
        JumpAttempt attempt = simulateJumpAgainst(
            tileMap,
            jumperMotionData(),
            physicsBodyData,
            glm::vec2(takeOffX, 9.0f * TileSize),
            1.0f,
            1.0f);
        if (!attempt.landed)
            continue;

        landedSomewhere = true;
        INFO("took off at " << takeOffX << ", came down at " << attempt.path.back().y);
        REQUIRE(std::fmod(attempt.path.back().y, static_cast<float>(TileSize)) == 0.0f);
    }

    REQUIRE(landedSomewhere);
}
