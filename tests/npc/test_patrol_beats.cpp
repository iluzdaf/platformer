#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/mantle_ability_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "conditions/asked.hpp"
#include "game/level.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

using namespace ledgeAndWall;

namespace
{
    constexpr float Slack = 8.0f;

    struct Reached
    {
        float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    };

    Reached whereItWent(Npc &npc, const Level &level, int steps, std::optional<glm::vec2> threat)
    {
        Reached reached;
        for (int step = 0; step < steps; ++step)
        {
            npc.beginFrame();
            npc.fixedUpdate(0.01f, level, threat);
            glm::vec2 foot = footOf(npc);
            reached.minX = std::min(reached.minX, foot.x);
            reached.maxX = std::max(reached.maxX, foot.x);
            reached.minY = std::min(reached.minY, foot.y);
            reached.maxY = std::max(reached.maxY, foot.y);
        }
        return reached;
    }

    NpcData aWalkerThatChases()
    {
        NpcData data = setupNpcData();
        BehaviorStateData chasing;
        chasing.name = "chase";
        chasing.does = ChaseBehaviorData{};
        data.stateMachineBehaviorData->states.push_back(chasing);
        data.stateMachineBehaviorData->transitions = {
            BehaviorTransitionData{
                "patrol", "chase", BehaviorWhenData{{{"threatNear", true}}}, 0.0f},
            BehaviorTransitionData{
                "chase", "patrol", BehaviorWhenData{{{"threatNear", false}}}, 2.0f}};
        data.facts["threatNear"] = false;
        return data;
    }

    NpcData aClimber()
    {
        NpcData data = setupNpcData();
        data.actorData.motionData.jumpAbilityData = JumpAbilityData{};
        data.actorData.motionData.wallHangAbilityData = WallHangAbilityData{};
        data.actorData.motionData.wallClimbAbilityData = WallClimbAbilityData{};
        return data;
    }

    NpcData aSpiderShapedClimber()
    {
        NpcData data = aClimber();
        data.actorData.size = glm::vec2(16.0f);
        data.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, 8.0f);
        data.actorData.physicsBodyData.colliderOffset = glm::vec2(4.0f, 8.0f);
        data.actorData.motionData.moveAbilityData = MoveAbilityData{90.0f};
        data.actorData.motionData.jumpAbilityData = JumpAbilityData{-320.0f};
        data.actorData.motionData.wallClimbAbilityData = WallClimbAbilityData{70.0f};
        data.actorData.motionData.mantleAbilityData = MantleAbilityData{};
        return data;
    }

    constexpr glm::ivec2 LeftBeat{6, GroundRow - 1};
    constexpr glm::ivec2 RightBeat{12, GroundRow - 1};
}

TEST_CASE("A patrol stays between its beats on a flat run", "[Npc][Patrol]")
{
    std::map<std::string, NpcData> walkers{{"walker", setupNpcData()}};
    NpcSpawnData spawn = patrolling("walker", LeftBeat, LeftBeat, RightBeat);
    Level level = levelWithALedgeAndAWall({spawn}, walkers);
    Npc npc(spawn, walkers.at("walker"));

    Reached reached = whereItWent(npc, level, 3000, std::nullopt);

    INFO("went x " << reached.minX << ".." << reached.maxX);
    REQUIRE(reached.minX >= feetOf(LeftBeat).x - Slack);
    REQUIRE(reached.maxX <= feetOf(RightBeat).x + Slack);
    REQUIRE(reached.maxX - reached.minX > 100.0f);
}

TEST_CASE("A patrol resumed after a chase stays between its beats", "[Npc][Patrol][Chase]")
{
    std::map<std::string, NpcData> chasers{{"chaser", aWalkerThatChases()}};
    NpcSpawnData spawn = patrolling("chaser", LeftBeat, LeftBeat, RightBeat);
    Level level = levelWithALedgeAndAWall({spawn}, chasers);
    Npc npc(spawn, chasers.at("chaser"));
    noticingAThreatWithin(npc, 64.0f);

    glm::vec2 you(feetOf(RightBeat).x + 48.0f, feetOf(RightBeat).y);
    whereItWent(npc, level, 400, you);
    REQUIRE(npc.stateName() == "chase");
    REQUIRE(std::abs(footOf(npc).x - you.x) < 16.0f);

    whereItWent(npc, level, 300, std::nullopt);
    REQUIRE(npc.stateName() == "patrol");

    Reached reached = whereItWent(npc, level, 3000, std::nullopt);

    INFO("went x " << reached.minX << ".." << reached.maxX);
    REQUIRE(reached.minX >= feetOf(LeftBeat).x - Slack);
    REQUIRE(reached.maxX <= feetOf(RightBeat).x + Slack);
}

TEST_CASE("A patrol whose beat ends part way up a wall climbs no higher", "[Npc][Patrol][Climb]")
{
    const glm::ivec2 partWayUp{1, 2};
    std::map<std::string, NpcData> climbers{{"climber", aClimber()}};
    NpcSpawnData spawn = patrolling("climber", LedgeRightEnd, LedgeRightEnd, partWayUp);
    Level level = levelWithALedgeAndAWall({spawn}, climbers);
    Npc npc(spawn, climbers.at("climber"));

    Reached reached = whereItWent(npc, level, 3000, std::nullopt);

    INFO(
        "went y " << reached.minY << ".." << reached.maxY << ", beat ends at "
                  << feetOf(partWayUp).y);
    REQUIRE(reached.minY <= feetOf(partWayUp).y + Slack);
    REQUIRE(reached.minY >= feetOf(partWayUp).y - Slack);
}

TEST_CASE("A patrol from a platform beat up to a wall beat keeps to both", "[Npc][Patrol][Climb]")
{
    const glm::ivec2 onThePlatform{14, RiseRow - 1};
    const glm::ivec2 upTheWall{RiseLastTile, 2};
    std::map<std::string, NpcData> climbers{{"climber", aSpiderShapedClimber()}};
    NpcSpawnData spawn = patrolling("climber", onThePlatform, onThePlatform, upTheWall);
    Level level = levelWithALedgeAndAWall({spawn}, climbers);
    Npc npc(spawn, climbers.at("climber"));

    Reached reached = whereItWent(npc, level, 3000, std::nullopt);

    INFO(
        "went x " << reached.minX << ".." << reached.maxX << " y " << reached.minY << ".."
                  << reached.maxY);
    REQUIRE(reached.minY <= feetOf(upTheWall).y + Slack);
    REQUIRE(reached.minX >= feetOf(onThePlatform).x - Slack);
}
