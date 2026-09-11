#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <filesystem>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "helpers/actors.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/levels.hpp"
#include "helpers/tiles.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "game/game_data.hpp"
#include "tile_map/tile_map_data.hpp"
#include "helpers/scripted_npcs.hpp"
#include "helpers/shipped.hpp"
#include "actor/actor_state.hpp"
#include "npc/npc.hpp"
#include "player/player.hpp"
#include "npc/striking_player.hpp"
#include "actor/health.hpp"
#include <vector>
#include <memory>
#include "npc/npc_spawn_data.hpp"
#include "game/noise.hpp"
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include <optional>
#include <variant>
#include "npc/npc_data.hpp"
#include "player/player_data.hpp"

namespace
{
    bool sleepsAtFirst(const NpcData &data)
    {
        const std::optional<StateMachineBehaviorData> &machine = data.stateMachineBehaviorData;
        return machine && !machine->states.empty() &&
               std::holds_alternative<IdleBehaviorData>(machine->states.front().does);
    }
}

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

            const NpcData &data = shippedNpcData().at(spawn.type);
            Npc npc(spawn, data);
            if (sleepsAtFirst(data))
            {
                REQUIRE(level.runBeneath(npc.profile(), spawn.feet).has_value());
                continue;
            }

            ScriptedNpcs scripts;
            scripts.script(npc);

            float startX = npc.body().position().x;
            stepNpc(npc, level, 400);

            REQUIRE(std::abs(npc.body().position().x - startX) > 1.0f);
        }
    }

    REQUIRE(placed > 0);
}

TEST_CASE("The shipped rat is safe to stand in while it patrols", "[Npc]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(SpawnTile));
    std::vector<std::unique_ptr<Npc>> rats;
    rats.push_back(std::make_unique<Npc>(spawnAt("rat", SpawnTile), shippedNpcData().at("rat")));

    strikePlayer(player, rats);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("The shipped rat, cornered, pounces through you and bites", "[Npc][Level][Pounce]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeLeftEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});
    std::vector<std::unique_ptr<Npc>> rats;
    rats.push_back(std::make_unique<Npc>(spawn, shippedNpcData().at("rat")));
    ScriptedNpcs scripts;
    scripts.script(*rats.front());
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(glm::vec2(feetOf(LedgeLeftEnd).x + 20.0f, surfaceOf(LedgeRow)));

    int bittenAt = -1;
    bool pouncingAtTheBite = false;
    for (int step = 0; step < 300 && bittenAt < 0; ++step)
    {
        rats.front()->beginFrame();
        rats.front()->fixedUpdate(0.01f, level, player.feet());
        strikePlayer(player, rats);
        if (player.health().points() < 3)
        {
            bittenAt = step;
            pouncingAtTheBite = rats.front()->stateName() == "pounce";
        }
    }

    INFO("bitten at step " << bittenAt);
    REQUIRE(bittenAt >= 0);
    REQUIRE(pouncingAtTheBite);
    REQUIRE(std::abs(footOf(*rats.front()).y - surfaceOf(LedgeRow)) < 8.0f);
}

TEST_CASE("The shipped spider walks up from the ground to a ledge and back", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("spider", OnTheGround, OnTheGround, LedgeLeftEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);

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

TEST_CASE("The shipped rat runs from the player and settles once it is gone", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling(
        "rat",
        glm::ivec2(6, GroundRow - 1),
        glm::ivec2(2, GroundRow - 1),
        glm::ivec2(17, GroundRow - 1));
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("rat"));
    ScriptedNpcs scripts;
    scripts.script(npc);

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

TEST_CASE("The shipped rat never freezes out in the open on its platform", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("rat"));
    ScriptedNpcs scripts;
    scripts.script(npc);

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

TEST_CASE("The shipped rat holds its ground while the player shares its platform", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("rat"));
    ScriptedNpcs scripts;
    scripts.script(npc);

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

TEST_CASE("The shipped rat does not shuffle on the spot once it is cornered", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("rat"));
    ScriptedNpcs scripts;
    scripts.script(npc);

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

TEST_CASE("The shipped rat pays no mind to a player on the platform below", "[Npc][Level]")
{
    NpcSpawnData spawn = patrolling("rat", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("rat"));
    ScriptedNpcs scripts;
    scripts.script(npc);

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

TEST_CASE("The shipped spider climbs the wall above the ledge", "[Npc][Level][Climb]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});

    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    const float theLedge = surfaceOf(LedgeRow);
    const float topOfTheFace = surfaceOf(1);

    float highest = footOf(npc).y;
    int reachedTheTopAt = -1;
    bool cameBackDown = false, showedTheClimb = false;
    for (int step = 0; step < 4000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        highest = std::min(highest, footOf(npc).y);
        if (reachedTheTopAt < 0 && highest <= topOfTheFace + 1.0f)
            reachedTheTopAt = step;
        if (reachedTheTopAt >= 0 && footOf(npc).y >= theLedge - 1.0f)
            cameBackDown = true;
        if (npc.state().currentAnimation == "climb")
            showedTheClimb = true;
    }

    INFO("highest foot reached " << highest << " at step " << reachedTheTopAt);
    REQUIRE(highest <= topOfTheFace + 1.0f);
    REQUIRE(cameBackDown);
    REQUIRE(reachedTheTopAt < 270);
    REQUIRE(showedTheClimb);
}

TEST_CASE("The level 6 spider keeps walking its beat", "[Npc][Level][Patrol]")
{
    LevelData level6 = readLevelData(assetPath("levels/level6.json"));
    Level level(level6, shippedPalettes(), PlayerData(), shippedNpcData(), shippedPickupData());
    NpcSpawnData spawn;
    for (const NpcSpawnData &placed : level6.npcs)
        if (placed.type == "spider" && placed.patrol)
            spawn = placed;
    REQUIRE(spawn.patrol.has_value());
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    int reachedTheFirst = 0, reachedTheSecond = 0;
    bool atTheFirst = false, atTheSecond = false;
    for (int step = 0; step < 3000; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
        glm::vec2 foot = footOf(npc);
        bool nowFirst = glm::distance(foot, spawn.patrol->from) < 8.0f;
        bool nowSecond = glm::distance(foot, spawn.patrol->to) < 8.0f;
        reachedTheFirst += nowFirst && !atTheFirst;
        reachedTheSecond += nowSecond && !atTheSecond;
        atTheFirst = nowFirst;
        atTheSecond = nowSecond;
    }

    INFO(
        "reached the first beat " << reachedTheFirst << " times and the second "
                                  << reachedTheSecond);
    REQUIRE(reachedTheFirst >= 2);
    REQUIRE(reachedTheSecond >= 2);
}

namespace
{
    constexpr glm::ivec2 OnTheLedge{3, LedgeRow - 1};

    bool hunting(const Npc &npc)
    {
        return npc.stateName() == "chase" || npc.stateName() == "pounce";
    }

    void stepNpcHunting(Npc &npc, const Level &level, glm::vec2 threatFeet, int steps)
    {
        for (int step = 0; step < steps; ++step)
        {
            npc.beginFrame();
            npc.fixedUpdate(0.01f, level, threatFeet);
        }
    }

}

TEST_CASE("The shipped spider gives chase when you step onto its ledge", "[Npc][Level][Chase]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you = feetOf(OnTheLedge);

    stepNpc(npc, level, 10);
    REQUIRE(npc.stateName() == "patrol");

    stepNpcHunting(npc, level, you, 10);
    REQUIRE(hunting(npc));

    stepNpcHunting(npc, level, you, 150);
    REQUIRE(std::abs(footOf(npc).x - you.x) <= 32.0f);
    REQUIRE(std::abs(footOf(npc).y - surfaceOf(LedgeRow)) <= 1.0f);
}

TEST_CASE("The shipped spider follows you down off its ledge", "[Npc][Level][Chase]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    stepNpcHunting(npc, level, feetOf(glm::ivec2(2, LedgeRow - 1)), 20);
    REQUIRE(hunting(npc));

    glm::vec2 below = feetOf(OnTheGround);
    bool reachedTheGroundHunting = false;
    for (int hunt = 0; hunt < 400 && !reachedTheGroundHunting; ++hunt)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, below);
        reachedTheGroundHunting =
            hunting(npc) && std::abs(footOf(npc).y - surfaceOf(GroundRow)) <= 1.0f;
    }

    REQUIRE(reachedTheGroundHunting);
}

TEST_CASE(
    "The shipped spider notices you from its wall and comes down",
    "[Npc][Level][Chase][Climb]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);

    const float wellAboveTheLedge = surfaceOf(LedgeRow) - 24.0f;
    const float wellBelowTheTop = surfaceOf(1) + 24.0f;
    int step = 0;
    for (; step < 400; ++step)
    {
        stepNpc(npc, level, 1);
        float y = footOf(npc).y;
        if (y < wellAboveTheLedge && y > wellBelowTheTop)
            break;
    }
    INFO("mid-wall at step " << step << ", foot y " << footOf(npc).y);
    REQUIRE(step < 400);

    glm::vec2 you = feetOf(glm::ivec2(2, LedgeRow - 1));
    stepNpcHunting(npc, level, you, 10);
    REQUIRE(hunting(npc));

    bool cameDownBesideYou = false;
    for (int hunt = 0; hunt < 300 && !cameDownBesideYou; ++hunt)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
        cameDownBesideYou = hunting(npc) && std::abs(footOf(npc).y - surfaceOf(LedgeRow)) <= 1.0f &&
                            std::abs(footOf(npc).x - you.x) <= 40.0f;
    }

    REQUIRE(cameDownBesideYou);
}

TEST_CASE("The shipped spider ignores you on the step below its ledge", "[Npc][Level][Chase]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you = feetOf(glm::ivec2(LedgeLastTile, StepRow - 1));

    INFO(
        "you at " << you.x << "," << you.y << ", spider at " << footOf(npc).x << ","
                  << footOf(npc).y);
    stepNpcHunting(npc, level, you, 20);

    REQUIRE(npc.stateName() == "patrol");
}

TEST_CASE("The shipped spider's patience outlasts its own climb on level 6", "[Npc][Level][Chase]")
{
    const NpcData &spider = shippedNpcData().at("spider");
    LevelData level6 = readLevelData(assetPath("levels/level6.json"));

    float tallestClimb = 0.0f;
    for (const NpcSpawnData &spawn : level6.npcs)
        if (spawn.type == "spider" && spawn.patrol)
            tallestClimb =
                std::max(tallestClimb, std::abs(spawn.patrol->from.y - spawn.patrol->to.y));
    REQUIRE(tallestClimb > 0.0f);

    float climbSpeed = spider.actorData.motionData.wallClimbAbilityData->climbSpeed;
    float givesUpAfter = 0.0f;
    for (const BehaviorTransitionData &transition : spider.stateMachineBehaviorData->transitions)
        if (transition.from == "chase" && transition.to == "patrol")
            givesUpAfter = transition.after;

    INFO(
        "climb of " << tallestClimb << " at " << climbSpeed << " takes "
                    << tallestClimb / climbSpeed);
    REQUIRE(givesUpAfter >= tallestClimb / climbSpeed);
}

TEST_CASE(
    "The shipped spider pounces when you are close, then waits out its cooldown",
    "[Npc][Level][Pounce]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you = feetOf(glm::ivec2(LedgeLastTile - 2, LedgeRow - 1));

    int pounces = 0;
    bool wasPouncing = false;
    int firstPounceAt = -1, secondPounceAt = -1;
    for (int step = 0; step < 500; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
        bool pouncing = npc.stateName() == "pounce";
        if (pouncing && !wasPouncing)
        {
            ++pounces;
            (firstPounceAt < 0 ? firstPounceAt : secondPounceAt) = step;
        }
        wasPouncing = pouncing;
    }

    INFO(
        "pounced " << pounces << " times, first at " << firstPounceAt << ", second at "
                   << secondPounceAt);
    REQUIRE(pounces >= 2);
    REQUIRE(secondPounceAt - firstPounceAt >= 200);
}

TEST_CASE("The shipped spider bites while pouncing and at no other time", "[Npc][Level][Pounce]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you = feetOf(glm::ivec2(LedgeLastTile - 2, LedgeRow - 1));

    REQUIRE_FALSE(npc.hurting().has_value());
    stepNpcHunting(npc, level, you, 20);
    REQUIRE(hunting(npc));
    REQUIRE(npc.hurting().has_value() == (npc.stateName() == "pounce"));

    bool bitWhilePouncing = false, bitOtherwise = false;
    for (int step = 0; step < 300; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
        if (npc.hurting())
            (npc.stateName() == "pounce" ? bitWhilePouncing : bitOtherwise) = true;
    }

    REQUIRE(bitWhilePouncing);
    REQUIRE_FALSE(bitOtherwise);
}

TEST_CASE(
    "The shipped spider's pounce lands on you, first time, at body height",
    "[Npc][Level][Pounce]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    std::vector<std::unique_ptr<Npc>> spiders;
    spiders.push_back(std::make_unique<Npc>(spawn, shippedNpcData().at("spider")));
    ScriptedNpcs scripts;
    scripts.script(*spiders.front());
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(glm::ivec2(2, LedgeRow - 1)));

    int bittenAt = -1;
    float footHeightAtTheBite = 0.0f;
    for (int step = 0; step < 600 && bittenAt < 0; ++step)
    {
        spiders.front()->beginFrame();
        spiders.front()->fixedUpdate(0.01f, level, player.feet());
        strikePlayer(player, spiders);
        if (player.health().points() < 3)
        {
            bittenAt = step;
            footHeightAtTheBite = surfaceOf(LedgeRow) - footOf(*spiders.front()).y;
        }
    }

    INFO(
        "bitten at step " << bittenAt << ", spider foot " << footHeightAtTheBite
                          << " above the ledge");
    REQUIRE(bittenAt >= 0);
    REQUIRE(bittenAt < 150);
    REQUIRE(footHeightAtTheBite <= 8.0f);
    REQUIRE(spiders.front()->stateName() == "pounce");
}

TEST_CASE("The shipped spider pounces only once you are within its reach", "[Npc][Level][Pounce]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeLeftEnd, LedgeRightEnd);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you(feetOf(LedgeLeftEnd).x + 20.0f, surfaceOf(LedgeRow));

    float reachAtThePounce = -1.0f;
    for (int step = 0; step < 600 && reachAtThePounce < 0.0f; ++step)
    {
        float before = glm::distance(footOf(npc), you);
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
        if (npc.stateName() == "pounce")
            reachAtThePounce = before;
    }

    INFO("pounced from " << reachAtThePounce);
    REQUIRE(reachAtThePounce >= 0.0f);
    REQUIRE(reachAtThePounce <= shippedNpcData().at("spider").tuning.at("reach") + 2.0f);
}

TEST_CASE(
    "The shipped boar charges when you land on its ground, is stunned at the wall, and sleeps "
    "again once you are gone",
    "[Npc][Level][Charge]")
{
    NpcSpawnData spawn = spawnAt("boar", glm::ivec2(9, GroundRow - 1));
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("boar"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you = feetOf(glm::ivec2(4, GroundRow - 1));
    for (int settle = 0; settle < 30; ++settle)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
    }
    REQUIRE(npc.stateName() == "sleep");

    std::vector<Noise> landing{{std::string(LandingNoise), you}};
    npc.beginFrame();
    npc.fixedUpdate(0.01f, level, you, landing);
    REQUIRE(npc.stateName() == "charge");

    int stunnedAt = -1;
    for (int step = 0; step < 300 && stunnedAt < 0; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
        if (npc.stateName() == "stunned")
            stunnedAt = step;
    }
    INFO("stunned at step " << stunnedAt << ", foot x " << footOf(npc).x);
    REQUIRE(stunnedAt >= 0);
    REQUIRE(footOf(npc).x < feetOf(glm::ivec2(2, GroundRow - 1)).x);

    for (int step = 0; step < 200; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
    }
    REQUIRE(npc.stateName() == "sleep");
}

TEST_CASE("The spider shows its pounce clip while its pounce state is on", "[ShippedNpcs]")
{
    NpcSpawnData spawn = patrolling("spider", LedgeRightEnd, LedgeRightEnd, TopOfTheWall);
    Level level = levelWithALedgeAndAWall({spawn});
    Npc npc(spawn, shippedNpcData().at("spider"));
    ScriptedNpcs scripts;
    scripts.script(npc);
    glm::vec2 you = feetOf(glm::ivec2(LedgeLastTile - 2, LedgeRow - 1));

    bool pouncedOnFilm = false, pouncedOffFilm = false, filmedElsewhere = false;
    for (int step = 0; step < 400; ++step)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you);
        bool inPounce = npc.stateName() == "pounce";
        bool onFilm = npc.state().currentAnimation == "pounce";
        if (inPounce && onFilm)
            pouncedOnFilm = true;
        if (inPounce && !onFilm)
            pouncedOffFilm = true;
        if (!inPounce && onFilm)
            filmedElsewhere = true;
    }

    REQUIRE(pouncedOnFilm);
    REQUIRE_FALSE(pouncedOffFilm);
    REQUIRE_FALSE(filmedElsewhere);
}

namespace
{
    Level aFloorWithNothingAtItsEnds(const std::vector<NpcSpawnData> &npcs)
    {
        TileMapData tileMapData;
        tileMapData.tilePalette = "default";
        tileMapData.indices =
            std::vector<std::vector<int>>(LedgeHeightTiles, std::vector<int>(LedgeWidthTiles, 0));
        for (int x = 0; x < LedgeWidthTiles; ++x)
            tileMapData.indices[GroundRow][x] = GroundTile;

        LevelData levelData;
        levelData.tileMapData = tileMapData;
        levelData.playerFeet = feetOf(glm::ivec2(1, GroundRow - 1));
        levelData.npcs = npcs;

        return Level(
            levelData,
            theOnlyPalette(ledgePalette()),
            loadGameData().playerData,
            shippedNpcData(),
            shippedPickupData());
    }

    struct Charged
    {
        int stunnedAt = -1;
        float footX = 0.0f;
    };

    Charged boarChargingFrom(glm::ivec2 boarTile, glm::ivec2 youTile)
    {
        NpcSpawnData spawn = spawnAt("boar", boarTile);
        Level level = aFloorWithNothingAtItsEnds({spawn});
        Npc npc(spawn, shippedNpcData().at("boar"));
        ScriptedNpcs scripts;
        scripts.script(npc);
        glm::vec2 you = feetOf(youTile);

        for (int settle = 0; settle < 30; ++settle)
        {
            npc.beginFrame();
            npc.fixedUpdate(0.01f, level, you);
        }
        REQUIRE(npc.stateName() == "sleep");

        std::vector<Noise> landing{{std::string(LandingNoise), you}};
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, you, landing);
        REQUIRE(npc.stateName() == "charge");

        Charged charged;
        for (int step = 0; step < 300 && charged.stunnedAt < 0; ++step)
        {
            npc.beginFrame();
            npc.fixedUpdate(0.01f, level, you);
            if (npc.stateName() == "stunned")
                charged.stunnedAt = step;
        }

        charged.footX = footOf(npc).x;
        return charged;
    }
}

TEST_CASE(
    "The shipped boar charging off the right end of the level is stunned there",
    "[Npc][Charge]")
{
    Charged charged = boarChargingFrom(
        glm::ivec2(14, GroundRow - 1), glm::ivec2(LedgeWidthTiles - 2, GroundRow - 1));

    INFO("stunned at step " << charged.stunnedAt << ", foot x " << charged.footX);
    REQUIRE(charged.stunnedAt >= 0);
    REQUIRE(charged.footX > feetOf(glm::ivec2(LedgeWidthTiles - 2, GroundRow - 1)).x);
}

TEST_CASE(
    "The shipped boar charging off the left end of the level is stunned there",
    "[Npc][Charge]")
{
    Charged charged = boarChargingFrom(glm::ivec2(5, GroundRow - 1), glm::ivec2(1, GroundRow - 1));

    INFO("stunned at step " << charged.stunnedAt << ", foot x " << charged.footX);
    REQUIRE(charged.stunnedAt >= 0);
    REQUIRE(charged.footX < feetOf(glm::ivec2(1, GroundRow - 1)).x);
}
