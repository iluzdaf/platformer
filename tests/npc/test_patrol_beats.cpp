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
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "game/level.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/levels.hpp"
#include "helpers/ledge_and_wall.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/scripted_npcs.hpp"
#include "player/player_data.hpp"
#include "helpers/tiles.hpp"
#include "helpers/palettes.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "state_machines/state_machine_data.hpp"
#include "conditions/when_data.hpp"
#include "helpers/creature_scripts.hpp"

using namespace ledge_and_wall;

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
            npc.fixedUpdate(0.01f, level, {.threatFeet = threat});
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
        NpcData data = thatPatrols(setupNpcData());
        data.script.path = aScriptThatRuns(
            std::vector<std::string>{
                "scripts/behaviors/patrol.lua", "scripts/behaviors/chase.lua"});
        BehaviorStateData chasing;
        chasing.name = "chase";
        chasing.does = ScriptedBehaviorData{"chase"};
        data.stateMachineBehaviorData->states.push_back(chasing);
        data.stateMachineBehaviorData->transitions = {
            TransitionData{"patrol", "chase", WhenData{{{"threatNear", true}}}, 0.0f},
            TransitionData{"chase", "patrol", WhenData{{{"threatNear", false}}}, 2.0f}};
        data.facts["threatNear"] = false;
        return data;
    }

    NpcData aClimber()
    {
        NpcData data = thatPatrols(setupNpcData());
        data.actorData.abilities.jump = JumpAbilityData{};
        data.actorData.abilities.wallHang = WallHangAbilityData{};
        data.actorData.abilities.wallClimb = WallClimbAbilityData{};
        return data;
    }

    NpcData aSpiderShapedClimber()
    {
        NpcData data = aClimber();
        data.actorData.size = glm::vec2(16.0f);
        data.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, 8.0f);
        data.actorData.physicsBodyData.colliderOffset = glm::vec2(4.0f, 8.0f);
        data.actorData.abilities.move = MoveAbilityData{90.0f};
        data.actorData.abilities.jump = JumpAbilityData{-320.0f};
        data.actorData.abilities.wallClimb = WallClimbAbilityData{70.0f};
        data.actorData.abilities.mantle = MantleAbilityData{};
        return data;
    }

    constexpr glm::ivec2 LeftBeat{6, GroundRow - 1};
    constexpr glm::ivec2 RightBeat{12, GroundRow - 1};
}

TEST_CASE("A patrol stays between its beats on a flat run", "[Npc][Patrol]")
{
    std::map<std::string, NpcData> walkers{{"walker", thatPatrols(setupNpcData())}};
    NpcSpawnData spawn = patrolling("walker", LeftBeat, LeftBeat, RightBeat);
    Level level = levelWithALedgeAndAWall({spawn}, walkers);
    Npc npc(spawn, walkers.at("walker"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    Reached reached = whereItWent(npc, level, 3000, std::nullopt);

    INFO("went x " << reached.minX << ".." << reached.maxX);
    REQUIRE(reached.minX >= feetOf(LeftBeat).x - Slack);
    REQUIRE(reached.maxX <= feetOf(RightBeat).x + Slack);
    REQUIRE(reached.maxX - reached.minX > 100.0f);
}

TEST_CASE("A patrol turns at a beat just past a step in its run", "[Npc][Patrol]")
{
    constexpr int Floor = 9;
    constexpr int Lower = 2;
    TileData lower;
    lower.solid = lower.grippable = true;
    lower.collider = TileColliderData{{0.0f, 3.0f}, {16.0f, 13.0f}};
    TileData solid = lower;
    solid.collider.reset();
    Placed laid;
    layColumn(laid, 0, 0, Floor);
    layColumn(laid, 13, 0, Floor);
    layRow(laid, Floor, 1, 3);
    layRow(laid, Floor, 4, 12, Lower);
    std::map<std::string, NpcData> walkers{{"walker", thatPatrols(setupNpcData())}};
    NpcSpawnData spawn = patrolling("walker", {2, Floor - 1}, {1, Floor - 1}, {4, Floor - 1});
    Level level(
        aLevelPlacing(laid, 14, 10, {2, Floor - 1}, {spawn}),
        theOnlyPalette(paletteOf({{EmptyTile, TileData{}}, {SolidTile, solid}, {Lower, lower}})),
        PlayerData(),
        walkers,
        {});
    Npc npc(spawn, walkers.at("walker"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    int turnedAtTheStep = 0;
    bool pastTheStep = false;
    float furthest = 0.0f;
    for (int step = 0; step < 3000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        glm::vec2 foot = footOf(npc);
        furthest = std::max(furthest, foot.x);
        turnedAtTheStep += pastTheStep && foot.x < 64.0f;
        pastTheStep = foot.x >= 64.0f;
    }

    INFO("went as far as " << furthest << " and turned back " << turnedAtTheStep << " times");
    REQUIRE(furthest <= spawn.patrol->to.x + Slack);
    REQUIRE(turnedAtTheStep >= 2);
}

TEST_CASE("An npc that steps off a ledge a little late still drops to the floor", "[Npc][Patrol]")
{
    constexpr int Floor = 9;
    TileData solid;
    solid.solid = solid.grippable = true;
    Placed laid;
    layRow(laid, 3, 0, 3);
    layRow(laid, Floor, 0, 11);
    NpcData quick = thatPatrols(setupNpcData());
    quick.actorData.abilities.move = MoveAbilityData{250.0f};
    quick.actorData.physicsBodyData.colliderSize = glm::vec2(5.0f, 13.0f);
    quick.actorData.physicsBodyData.colliderOffset = glm::vec2(5.5f, 3.0f);
    std::map<std::string, NpcData> walkers{{"walker", quick}};
    NpcSpawnData spawn = patrolling("walker", {3, 2}, {8, Floor - 1}, {1, 2});
    spawn.feet = glm::vec2(62.6f, 48.0f);
    Level level(
        aLevelPlacing(laid, 12, 12, {1, 2}, {spawn}),
        theOnlyPalette(paletteOf({{EmptyTile, TileData{}}, {SolidTile, solid}})),
        PlayerData(),
        walkers,
        {});
    Npc npc(spawn, walkers.at("walker"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    bool onTheFloor = false;
    for (int step = 0; step < 300 && !onTheFloor; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        onTheFloor = npc.onGround() && footOf(npc).y > 100.0f;
    }

    INFO("ended at " << footOf(npc).x << "," << footOf(npc).y);
    REQUIRE(onTheFloor);
}

TEST_CASE("A patrol resumed after a chase stays between its beats", "[Npc][Patrol][Chase]")
{
    std::map<std::string, NpcData> chasers{{"chaser", aWalkerThatChases()}};
    NpcSpawnData spawn = patrolling("chaser", LeftBeat, LeftBeat, RightBeat);
    Level level = levelWithALedgeAndAWall({spawn}, chasers);
    Npc npc(spawn, chasers.at("chaser"));
    ScriptedNpcs scripts;
    scripts.script(npc);
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
    ScriptedNpcs scripts;
    scripts.script(npc);

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
    ScriptedNpcs scripts;
    scripts.script(npc);

    Reached reached = whereItWent(npc, level, 3000, std::nullopt);

    INFO(
        "went x " << reached.minX << ".." << reached.maxX << " y " << reached.minY << ".."
                  << reached.maxY);
    REQUIRE(reached.minY <= feetOf(upTheWall).y + Slack);
    REQUIRE(reached.minX >= feetOf(onThePlatform).x - Slack);
}
