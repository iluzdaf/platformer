#include <catch2/catch_test_macros.hpp>
#include <string>
#include "actor/abilities/pounce_ability_data.hpp"
#include <optional>
#include "actor/actor_behavior_context.hpp"
#include "helpers/behaviour_context.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    StateMachineBehaviorData setupData(float threatWithin = 48.0f, float calmDown = 0.0f)
    {
        BehaviorStateData patrolling;
        patrolling.name = "patrol";
        patrolling.patrolBehaviorData = PatrolBehaviorData{};

        BehaviorStateData fleeing;
        fleeing.name = "flee";
        fleeing.fleeBehaviorData = FleeBehaviorData{};

        BehaviorTransitionData alarmed;
        alarmed.from = "patrol";
        alarmed.to = "flee";
        alarmed.threatWithin = threatWithin;

        BehaviorTransitionData calmed;
        calmed.from = "flee";
        calmed.to = "patrol";
        calmed.threatBeyond = threatWithin * 2.0f;
        calmed.after = calmDown;

        return {{patrolling, fleeing}, {alarmed, calmed}};
    }
}

TEST_CASE("Starts in the first state it was given", "[StateMachineBehavior]")
{
    StateMachineBehavior behavior(setupData());

    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Switches state once the threat is close enough", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData());

    behavior.decide(0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Stays alarmed while the threat is only a little further off", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData());

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(130.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Calms down once the threat is well clear", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData());

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Waits out the calm down before going back to patrol", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(48.0f, 1.0f));

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 50; ++step)
        behavior.decide(
            0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 60; ++step)
        behavior.decide(
            0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("A threat that comes back resets the calm down", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(48.0f, 1.0f));

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    for (int step = 0; step < 90; ++step)
        behavior.decide(
            0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    for (int step = 0; step < 90; ++step)
        behavior.decide(
            0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 20; ++step)
        behavior.decide(
            0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Runs the state it is in, not the one it left", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData());

    InputIntentions patrolling = behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(patrolling.direction.x == -1.0f);

    InputIntentions fleeing = behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(fleeing.direction.x == 1.0f);
}

TEST_CASE("Given no states at all it asks for nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(StateMachineBehaviorData{});

    InputIntentions inputIntentions = behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 0.0f);
}

TEST_CASE("Holds a state while the threat shares its run", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    navigationGraph.addNode(5, {192.0f, 288.0f});

    StateMachineBehaviorData data = setupData();
    data.transitions.at(1) =
        BehaviorTransitionData{"flee", "patrol", std::nullopt, std::nullopt, false, 0.0f};

    StateMachineBehavior behavior(data);

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(192.0f, 288.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Ignores a threat that is close by but not on its ground", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    navigationGraph.addNode(5, {192.0f, 224.0f});

    StateMachineBehaviorData data = setupData();
    data.transitions.at(0) =
        BehaviorTransitionData{"patrol", "flee", 48.0f, std::nullopt, true, 0.0f};

    StateMachineBehavior behavior(data);

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(192.0f, 224.0f)));
    REQUIRE(behavior.getStateName() == "patrol");

    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("A state with nothing to do asks for nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData idle;
    idle.name = "idle";
    StateMachineBehaviorData data;
    data.states = {idle};
    StateMachineBehavior behavior(data, std::nullopt);

    InputIntentions asked =
        behavior.decide(0.016f, standingAt(navigationGraph, {0.0f, 192.0f}, std::nullopt));

    REQUIRE(behavior.getStateName() == "idle");
    REQUIRE(asked.direction.x == 0.0f);
    REQUIRE_FALSE(asked.jumpRequested);
}

TEST_CASE("A transition to a state it does not have is ignored", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehaviorData data = setupData();
    BehaviorTransitionData haunted;
    haunted.from = "flee";
    haunted.to = "ghost";
    haunted.threatWithin = 48.0f;
    data.transitions.push_back(haunted);
    StateMachineBehavior behavior(data, std::nullopt);
    ActorBehaviorContext threatened =
        standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(8.0f, 192.0f));

    behavior.decide(0.016f, threatened);
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.016f, threatened);

    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Given no states at all, resetting is nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(StateMachineBehaviorData{}, std::nullopt);

    behavior.reset();

    REQUIRE(
        behavior.decide(0.016f, standingAt(navigationGraph, {0.0f, 192.0f}, std::nullopt))
            .direction.x == 0.0f);
}

TEST_CASE("A state told to chase closes on the threat", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();

    BehaviorStateData chasing;
    chasing.name = "chase";
    chasing.chaseBehaviorData = ChaseBehaviorData{};
    StateMachineBehavior behavior(StateMachineBehaviorData{{chasing}, {}});

    InputIntentions closingIn = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(384.0f, 192.0f)));

    REQUIRE(behavior.getStateName() == "chase");
    REQUIRE(closingIn.direction.x == 1.0f);
}

namespace
{
    StateMachineBehaviorData aChaseThatPounces(float cooldown)
    {
        BehaviorStateData chasing;
        chasing.name = "chase";
        chasing.chaseBehaviorData = ChaseBehaviorData{};

        BehaviorStateData pouncing;
        pouncing.name = "pounce";
        pouncing.attackBehaviorData = AttackBehaviorData{std::string(PounceAttack)};
        pouncing.cooldown = cooldown;

        BehaviorTransitionData close;
        close.from = "chase";
        close.to = "pounce";
        close.threatWithin = 48.0f;

        BehaviorTransitionData landed;
        landed.from = "pounce";
        landed.to = "chase";
        landed.onGround = true;
        landed.after = 0.05f;

        return {{chasing, pouncing}, {close, landed}};
    }
}

TEST_CASE("A state on cooldown is not re-entered until it has passed", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aChaseThatPounces(1.0f));
    ActorBehaviorContext close =
        standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(200.0f, 192.0f));
    auto pouncesIn = [&](int steps)
    {
        int pounces = 0;
        for (int step = 0; step < steps; ++step)
        {
            behavior.decide(0.01f, close);
            pounces += behavior.getStateName() == "pounce";
        }
        return pounces;
    };

    REQUIRE(pouncesIn(1) == 1);
    REQUIRE(pouncesIn(10) > 0);
    REQUIRE(behavior.getStateName() == "chase");

    REQUIRE(pouncesIn(80) == 0);
    REQUIRE(behavior.secondsSinceLeaving("pounce") < 1.0f);

    REQUIRE(pouncesIn(40) > 0);
}

TEST_CASE("A transition can wait for the ground", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aChaseThatPounces(1.0f));
    behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(200.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "pounce");

    ActorBehaviorContext inTheAir = airborneAt(navigationGraph, {196.0f, 180.0f});
    inTheAir.threatFeet = glm::vec2(200.0f, 192.0f);
    for (int step = 0; step < 20; ++step)
        behavior.decide(0.01f, inTheAir);
    REQUIRE(behavior.getStateName() == "pounce");

    for (int step = 0; step < 10; ++step)
        behavior.decide(
            0.01f, standingAt(navigationGraph, {196.0f, 192.0f}, glm::vec2(200.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "chase");
}

TEST_CASE("A state told to pounce leaps at the threat", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.attackBehaviorData = AttackBehaviorData{std::string(PounceAttack)};
    StateMachineBehavior behavior(StateMachineBehaviorData{{pouncing}, {}});

    InputIntentions leap = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    REQUIRE(leap.attack == PounceAttack);
    REQUIRE(leap.direction.x == 1.0f);
}
