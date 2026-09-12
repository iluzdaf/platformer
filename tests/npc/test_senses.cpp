#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"
#include "game/level.hpp"
#include "game/noise.hpp"
#include "helpers/tile_positions.hpp"
#include "helpers/levels.hpp"
#include "helpers/ledge_and_wall.hpp"
#include "helpers/npc_fixtures.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

using namespace ledge_and_wall;

namespace
{
    NpcData aListener()
    {
        NpcData listener = setupNpcData();
        BehaviorStateData listening;
        listening.name = "listen";
        listening.does = IdleBehaviorData{};
        listener.stateMachineBehaviorData = StateMachineBehaviorData{{listening}, {}};
        listener.facts["heard"] = false;
        listener.facts["near"] = false;
        listener.facts["mood"] = std::string("calm");
        listener.tuning["range"] = 48.0f;
        return listener;
    }

    NpcData aSleeperThatWakesToALanding()
    {
        NpcData sleeper = aListener();
        BehaviorStateData sleeping;
        sleeping.name = "sleep";
        sleeping.does = IdleBehaviorData{};
        BehaviorStateData charging;
        charging.name = "charge";
        charging.does = ChaseBehaviorData{};
        BehaviorTransitionData woken;
        woken.from = "sleep";
        woken.to = "charge";
        woken.when["heard"] = true;
        sleeper.stateMachineBehaviorData = StateMachineBehaviorData{{sleeping, charging}, {woken}};
        return sleeper;
    }

    void tick(Npc &npc, const Level &level, std::optional<glm::vec2> threat = std::nullopt)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level, threat);
    }
}

TEST_CASE("A creature says where its threat is, how far, and whether it stands", "[Senses]")
{
    NpcData listener = aListener();
    NpcSpawnData spawn = spawnAt("listener", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"listener", listener}});
    Npc npc(spawn, listener);

    tick(npc, level);
    REQUIRE_FALSE(npc.threatFeet().has_value());

    glm::vec2 you = npc.feet() + glm::vec2(30.0f, 0.0f);
    tick(npc, level, you);
    REQUIRE(npc.threatFeet() == you);
    REQUIRE(npc.distanceTo(you) == glm::distance(npc.feet(), you));
    REQUIRE(npc.distanceTo(npc.feet() + glm::vec2(0.0f, 30.0f)) == 30.0f);
    REQUIRE(npc.onGround());
}

TEST_CASE("A creature knows whether a point shares its surface", "[Senses]")
{
    NpcData listener = aListener();
    NpcSpawnData spawn = spawnAt("listener", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"listener", listener}});
    Npc npc(spawn, listener);
    for (int settle = 0; settle < 30; ++settle)
        tick(npc, level);

    REQUIRE(npc.onSameSurfaceAs(feetOf(glm::ivec2(6, GroundRow - 1))));
    REQUIRE_FALSE(npc.onSameSurfaceAs(feetOf(LedgeRightEnd)));
}

TEST_CASE("A creature says whether a point has it cornered", "[Senses]")
{
    NpcData listener = aListener();
    NpcSpawnData spawn = spawnAt("listener", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"listener", listener}});
    Npc npc(spawn, listener);
    for (int settle = 0; settle < 30; ++settle)
        tick(npc, level);

    glm::vec2 open = npc.feet();
    REQUIRE_FALSE(npc.corneredBy(open + glm::vec2(12.0f, 0.0f)));

    npc.standAt(glm::vec2(20.0f, open.y));
    tick(npc, level);
    REQUIRE(npc.corneredBy(npc.feet() + glm::vec2(12.0f, 0.0f)));
}

TEST_CASE("Before its first tick a creature cannot be asked about the ground", "[Senses]")
{
    NpcData listener = aListener();
    Npc npc(spawnAt("listener", OnTheGround), listener);

    REQUIRE_THROWS_AS(npc.onSameSurfaceAs(glm::vec2(0.0f)), std::runtime_error);
    REQUIRE_THROWS_AS(npc.corneredBy(glm::vec2(0.0f)), std::runtime_error);
}

TEST_CASE("A fact said lasts, and an event lasts the tick it is heard in", "[Senses]")
{
    NpcData listener = aListener();
    NpcSpawnData spawn = spawnAt("listener", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"listener", listener}});
    Npc npc(spawn, listener);
    std::vector<Asked> heardDuringTheTick;
    npc.onNoise.connect([&npc](const Noise &) { npc.event("heard", true); });
    npc.onTick.connect([&](float) { heardDuringTheTick.push_back(npc.fact("heard")); });

    npc.fact("mood", std::string("angry"));
    tick(npc, level);
    REQUIRE(heardDuringTheTick == std::vector<Asked>{false});
    REQUIRE(npc.facts().at("mood") == Asked{std::string("angry")});

    std::vector<Noise> aLanding{{std::string(LandingNoise), npc.feet()}};
    npc.beginFrame();
    npc.fixedUpdate(0.01f, level, std::nullopt, aLanding);
    REQUIRE(heardDuringTheTick == std::vector<Asked>{false, true});
    REQUIRE(npc.facts().at("heard") == Asked{false});

    tick(npc, level);
    REQUIRE(heardDuringTheTick == std::vector<Asked>{false, true, false});
}

TEST_CASE("What an event said lingers after the tick, and then is gone", "[Senses]")
{
    NpcData listener = aListener();
    NpcSpawnData spawn = spawnAt("listener", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"listener", listener}});
    Npc npc(spawn, listener);
    npc.onNoise.connect([&npc](const Noise &) { npc.event("heard", true); });

    tick(npc, level);
    REQUIRE(npc.saidLately().all().empty());

    std::vector<Noise> aLanding{{std::string(LandingNoise), npc.feet()}};
    npc.beginFrame();
    npc.fixedUpdate(0.01f, level, std::nullopt, aLanding);

    REQUIRE(npc.facts().at("heard") == Asked{false});
    REQUIRE(npc.saidLately().all().at("heard").value == Asked{true});

    for (int step = 0; step < 30; ++step)
        tick(npc, level);

    REQUIRE(npc.saidLately().all().contains("heard"));

    for (int step = 0; step < 30; ++step)
        tick(npc, level);

    REQUIRE(npc.saidLately().all().empty());
}

TEST_CASE("A fact nobody declared, or of the wrong kind, is refused", "[Senses]")
{
    NpcData listener = aListener();
    Npc npc(spawnAt("listener", OnTheGround), listener);

    REQUIRE_THROWS_AS(npc.fact("herd", true), std::runtime_error);
    REQUIRE_THROWS_AS(npc.fact("near", 3.0f), std::runtime_error);
    REQUIRE_THROWS_AS(npc.event("herd", true), std::runtime_error);
    REQUIRE_THROWS_AS(npc.event("near", 3.0f), std::runtime_error);
    REQUIRE_THROWS_AS(npc.fact("herd"), std::runtime_error);
    REQUIRE(npc.facts() == listener.facts);
}

TEST_CASE("A fact the engine answers cannot be declared", "[Senses]")
{
    NpcData listener = aListener();
    listener.facts["onGround"] = true;

    REQUIRE_THROWS_AS(Npc(spawnAt("listener", OnTheGround), listener), std::runtime_error);
}

TEST_CASE("Tuning is read by name, and a name nobody tuned is refused", "[Senses]")
{
    NpcData listener = aListener();
    Npc npc(spawnAt("listener", OnTheGround), listener);

    REQUIRE(npc.tuning("range") == 48.0f);
    REQUIRE_THROWS_AS(npc.tuning("patience"), std::runtime_error);
}

TEST_CASE(
    "A sleeper wakes the tick it hears something land on its surface, and not for a landing "
    "elsewhere",
    "[Senses]")
{
    NpcData sleeper = aSleeperThatWakesToALanding();
    NpcSpawnData spawn = spawnAt("sleeper", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"sleeper", sleeper}});
    Npc npc(spawn, sleeper);
    npc.onNoise.connect(
        [&npc](const Noise &noise)
        {
            if (noise.kind == LandingNoise && npc.onSameSurfaceAs(noise.at))
                npc.event("heard", true);
        });
    for (int settle = 0; settle < 30; ++settle)
        tick(npc, level);
    REQUIRE(npc.stateName() == "sleep");

    glm::vec2 you = feetOf(glm::ivec2(6, GroundRow - 1));
    std::vector<Noise> onTheLedge{{std::string(LandingNoise), feetOf(LedgeRightEnd)}};
    npc.beginFrame();
    npc.fixedUpdate(0.01f, level, you, onTheLedge);
    REQUIRE(npc.stateName() == "sleep");

    std::vector<Noise> onMyGround{{std::string(LandingNoise), you}};
    npc.beginFrame();
    npc.fixedUpdate(0.01f, level, you, onMyGround);
    REQUIRE(npc.stateName() == "charge");
}
