#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "game/level.hpp"
#include "game/noise.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

namespace
{
    NpcData aSleeperThatWakesToALanding()
    {
        NpcData sleeper = setupNpcData();
        BehaviorStateData sleeping;
        sleeping.name = "sleep";
        sleeping.does = IdleBehaviorData{};
        BehaviorStateData charging;
        charging.name = "charge";
        charging.does = ChaseBehaviorData{};
        BehaviorTransitionData woken;
        woken.from = "sleep";
        woken.to = "charge";
        woken.when["landingOnMySurface"] = true;
        sleeper.stateMachineBehaviorData = StateMachineBehaviorData{{sleeping, charging}, {woken}};
        return sleeper;
    }
}

TEST_CASE(
    "A sleeper wakes the tick after something lands on its surface, and not for a landing "
    "elsewhere",
    "[Hearing]")
{
    NpcData sleeper = aSleeperThatWakesToALanding();
    NpcSpawnData spawn = spawnAt("sleeper", OnTheGround);
    Level level = levelWithALedgeAndAWall({spawn}, {{"sleeper", sleeper}});
    Npc npc(spawn, sleeper);
    for (int settle = 0; settle < 30; ++settle)
    {
        npc.beginFrame();
        npc.fixedUpdate(0.01f, level);
    }
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
