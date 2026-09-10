#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/state_machine_shown.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"

namespace
{
    BehaviorStateData aState(const std::string &name)
    {
        BehaviorStateData state;
        state.name = name;
        return state;
    }

    BehaviorTransitionData aTransition(const std::string &from, const std::string &to)
    {
        BehaviorTransitionData transition;
        transition.from = from;
        transition.to = to;
        return transition;
    }
}

TEST_CASE("A state says what it does in words", "[StateMachineShown]")
{
    BehaviorStateData patrolling = aState("patrol");
    patrolling.does = PatrolBehaviorData{};
    REQUIRE(behaviourOf(patrolling) == "patrol");

    BehaviorStateData fleeing = aState("flee");
    fleeing.does = FleeBehaviorData{};
    REQUIRE(behaviourOf(fleeing) == "flee");

    BehaviorStateData chasing = aState("chase");
    ChaseBehaviorData chase;
    chase.standoff = 28.0f;
    chasing.does = chase;
    REQUIRE(behaviourOf(chasing) == "chase, standoff 28");

    BehaviorStateData pouncing = aState("pounce");
    pouncing.does = AttackBehaviorData{"pounce"};
    pouncing.cooldown = 2.0f;
    REQUIRE(behaviourOf(pouncing) == "attack with pounce, cooldown 2 s");

    REQUIRE(behaviourOf(aState("idle")) == "does nothing");
}

TEST_CASE("A transition says when it fires, every condition in one line", "[StateMachineShown]")
{
    BehaviorTransitionData transition = aTransition("flee", "pounce");
    transition.when["threatInReach"] = true;
    transition.when["threatOnMySurface"] = true;
    transition.when["cornered"] = true;
    REQUIRE(whenOf(transition) == "cornered, threatInReach, threatOnMySurface");

    BehaviorTransitionData back = aTransition("flee", "patrol");
    back.when["threatOnMySurface"] = false;
    back.after = 1.5f;
    REQUIRE(whenOf(back) == "not threatOnMySurface, after 1.5 s");

    BehaviorTransitionData landed = aTransition("pounce", "chase");
    landed.when["onGround"] = true;
    landed.after = 0.1f;
    REQUIRE(whenOf(landed) == "on ground, after 0.1 s");

    BehaviorTransitionData angry = aTransition("chase", "patrol");
    angry.when["hits"] = 3.0f;
    angry.when["mood"] = std::string("angry");
    angry.when["onGround"] = false;
    REQUIRE(whenOf(angry) == "hits 3, mood \"angry\", in the air");

    REQUIRE(whenOf(aTransition("a", "b")) == "always");
}

TEST_CASE("States sit on a ring, the first at the top, going clockwise", "[StateMachineShown]")
{
    std::vector<glm::vec2> ring = aRingOf(4, glm::vec2(100.0f, 50.0f), 10.0f);

    REQUIRE(ring.size() == 4);
    REQUIRE(ring[0].x == Catch::Approx(100.0f).margin(0.001f));
    REQUIRE(ring[0].y == Catch::Approx(40.0f).margin(0.001f));
    REQUIRE(ring[1].x == Catch::Approx(110.0f).margin(0.001f));
    REQUIRE(ring[1].y == Catch::Approx(50.0f).margin(0.001f));
    for (const glm::vec2 &position : ring)
        REQUIRE(glm::distance(position, glm::vec2(100.0f, 50.0f)) == Catch::Approx(10.0f));
}

TEST_CASE("A lone state sits in the middle", "[StateMachineShown]")
{
    std::vector<glm::vec2> ring = aRingOf(1, glm::vec2(100.0f, 50.0f), 10.0f);

    REQUIRE(ring.size() == 1);
    REQUIRE(ring[0] == glm::vec2(100.0f, 50.0f));
}

TEST_CASE("A state is found by its name", "[StateMachineShown]")
{
    StateMachineBehaviorData machine{{aState("patrol"), aState("chase")}, {}};

    REQUIRE(indexOfState(machine, "chase") == 1);
    REQUIRE_FALSE(indexOfState(machine, "pounce").has_value());
}

TEST_CASE("A transition knows whether its reverse exists", "[StateMachineShown]")
{
    StateMachineBehaviorData machine{
        {aState("patrol"), aState("chase"), aState("pounce")},
        {aTransition("patrol", "chase"),
         aTransition("chase", "patrol"),
         aTransition("chase", "pounce")}};

    REQUIRE(goesBothWays(machine, machine.transitions[0]));
    REQUIRE(goesBothWays(machine, machine.transitions[1]));
    REQUIRE_FALSE(goesBothWays(machine, machine.transitions[2]));
}

TEST_CASE("A selection past the end of the machine is no selection", "[StateMachineShown]")
{
    StateMachineBehaviorData machine{
        {aState("patrol"), aState("chase")}, {aTransition("patrol", "chase")}};

    REQUIRE(stillAmong(showingState(1), machine) == showingState(1));
    REQUIRE(stillAmong(showingState(2), machine) == MachineShown{});
    REQUIRE(stillAmong(showingTransition(0), machine) == showingTransition(0));
    REQUIRE(stillAmong(showingTransition(1), machine) == MachineShown{});
    REQUIRE(stillAmong(MachineShown{}, machine) == MachineShown{});
}

TEST_CASE(
    "A point on the curve is no distance from it, one beside it is as far as it sits",
    "[StateMachineShown]")
{
    glm::vec2 start(0.0f, 0.0f), control(50.0f, 40.0f), end(100.0f, 0.0f);

    REQUIRE(onCurve(0.5f, start, control, end) == glm::vec2(50.0f, 20.0f));
    REQUIRE(
        distanceToCurve(glm::vec2(50.0f, 20.0f), start, control, end) ==
        Catch::Approx(0.0f).margin(0.01f));
    REQUIRE(
        distanceToCurve(glm::vec2(50.0f, 60.0f), start, control, end) ==
        Catch::Approx(40.0f).margin(0.5f));
    REQUIRE(distanceToCurve(start, start, control, end) == Catch::Approx(0.0f).margin(0.01f));
}
