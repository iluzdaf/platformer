#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/decided.hpp"
#include <optional>
#include "actor/actor_behavior_context.hpp"
#include "helpers/behaviour_context.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    StateMachineBehaviorData setupData(float calmDown = 0.0f)
    {
        BehaviorStateData patrolling;
        patrolling.name = "patrol";
        patrolling.does = PatrolBehaviorData{};

        BehaviorStateData fleeing;
        fleeing.name = "flee";
        fleeing.does = FleeBehaviorData{};

        BehaviorTransitionData alarmed;
        alarmed.from = "patrol";
        alarmed.to = "flee";
        alarmed.when["near"] = true;

        BehaviorTransitionData calmed;
        calmed.from = "flee";
        calmed.to = "patrol";
        calmed.when["near"] = false;
        calmed.after = calmDown;

        return {{patrolling, fleeing}, {alarmed, calmed}};
    }

    Facts knowingNear()
    {
        Facts facts;
        facts["near"] = false;
        return facts;
    }

    Facts saying(const std::string &name, const Asked &value)
    {
        Facts facts = knowingNear();
        facts[name] = value;
        return facts;
    }

    ActorBehaviorContext told(const Facts &facts, ActorBehaviorContext context)
    {
        context.facts = &facts;
        return context;
    }
}

TEST_CASE("Starts in the first state it was given", "[StateMachineBehavior]")
{
    StateMachineBehavior behavior(setupData(), std::nullopt, knowingNear());

    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Switches state once the fact it asks for is said", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(), std::nullopt, knowingNear());
    Facts calm = knowingNear();
    Facts near = saying("near", true);

    behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "patrol");

    behavior.decide(0.01f, told(near, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Calms down once the fact is unsaid", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(), std::nullopt, knowingNear());
    Facts calm = knowingNear();
    Facts near = saying("near", true);

    behavior.decide(0.01f, told(near, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Waits out the calm down before going back to patrol", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(1.0f), std::nullopt, knowingNear());
    Facts calm = knowingNear();
    Facts near = saying("near", true);

    behavior.decide(0.01f, told(near, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 50; ++step)
        behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 60; ++step)
        behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("A fact said again resets the calm down", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(1.0f), std::nullopt, knowingNear());
    Facts calm = knowingNear();
    Facts near = saying("near", true);

    behavior.decide(0.01f, told(near, standingAt(navigationGraph, {192.0f, 192.0f})));

    for (int step = 0; step < 90; ++step)
        behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));

    behavior.decide(0.01f, told(near, standingAt(navigationGraph, {192.0f, 192.0f})));

    for (int step = 0; step < 90; ++step)
        behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 20; ++step)
        behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Runs the state it is in, not the one it left", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(), std::nullopt, knowingNear());
    Facts calm = knowingNear();
    Facts near = saying("near", true);

    InputIntentions patrolling = behavior.decide(
        0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f))));
    REQUIRE(patrolling.direction.x == -1.0f);

    InputIntentions fleeing = behavior.decide(
        0.01f,
        told(near, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f))));
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

TEST_CASE(
    "A number fact holds when it is equal, and a name when it matches",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData idling;
    idling.name = "idle";
    BehaviorStateData angry;
    angry.name = "angry";
    BehaviorTransitionData provoked;
    provoked.from = "idle";
    provoked.to = "angry";
    provoked.when["hits"] = 2.0f;
    provoked.when["mood"] = std::string("sour");
    Facts facts;
    facts["hits"] = 0.0f;
    facts["mood"] = std::string("calm");
    StateMachineBehavior behavior(
        StateMachineBehaviorData{{idling, angry}, {provoked}}, std::nullopt, facts);

    facts["hits"] = 2.0f;
    behavior.decide(0.01f, told(facts, standingAt(navigationGraph, {0.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "idle");

    facts["mood"] = std::string("sour");
    behavior.decide(0.01f, told(facts, standingAt(navigationGraph, {0.0f, 192.0f})));
    REQUIRE(behavior.getStateName() == "angry");
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
    haunted.when["near"] = true;
    data.transitions.push_back(haunted);
    StateMachineBehavior behavior(data, std::nullopt, knowingNear());
    Facts near = saying("near", true);
    ActorBehaviorContext threatened =
        told(near, standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(8.0f, 192.0f)));

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
    chasing.does = ChaseBehaviorData{};
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
        chasing.does = ChaseBehaviorData{};

        BehaviorStateData pouncing;
        pouncing.name = "pounce";
        pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
        pouncing.cooldown = cooldown;

        BehaviorTransitionData close;
        close.from = "chase";
        close.to = "pounce";
        close.when["near"] = true;

        BehaviorTransitionData landed;
        landed.from = "pounce";
        landed.to = "chase";
        landed.when["onGround"] = true;
        landed.after = 0.05f;

        return {{chasing, pouncing}, {close, landed}};
    }
}

TEST_CASE("A state on cooldown is not re-entered until it has passed", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aChaseThatPounces(1.0f), std::nullopt, knowingNear());
    Facts near = saying("near", true);
    ActorBehaviorContext close =
        told(near, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(200.0f, 192.0f)));
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

TEST_CASE(
    "A transition can wait for the ground, which the engine answers",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aChaseThatPounces(1.0f), std::nullopt, knowingNear());
    Facts near = saying("near", true);
    behavior.decide(
        0.01f,
        told(near, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(200.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "pounce");

    ActorBehaviorContext inTheAir = told(near, airborneAt(navigationGraph, {196.0f, 180.0f}));
    inTheAir.threatFeet = glm::vec2(200.0f, 192.0f);
    for (int step = 0; step < 20; ++step)
        behavior.decide(0.01f, inTheAir);
    REQUIRE(behavior.getStateName() == "pounce");

    for (int step = 0; step < 10; ++step)
        behavior.decide(
            0.01f,
            told(near, standingAt(navigationGraph, {196.0f, 192.0f}, glm::vec2(200.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "chase");
}

TEST_CASE("A state told to pounce leaps at the threat", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    StateMachineBehavior behavior(StateMachineBehaviorData{{pouncing}, {}});

    InputIntentions leap = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    REQUIRE(leap.attack == PounceAttack);
    REQUIRE(leap.direction.x == 1.0f);
}

TEST_CASE(
    "A fleeing creature pounces when told the threat is in reach, then flees again once it lands",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData fleeing;
    fleeing.name = "flee";
    fleeing.does = FleeBehaviorData{};
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    pouncing.cooldown = 1.0f;
    BehaviorTransitionData tooClose;
    tooClose.from = "flee";
    tooClose.to = "pounce";
    tooClose.when["inReach"] = true;
    BehaviorTransitionData landed;
    landed.from = "pounce";
    landed.to = "flee";
    landed.when["onGround"] = true;
    landed.after = 0.05f;
    Facts facts;
    facts["inReach"] = false;
    StateMachineBehavior behavior({{fleeing, pouncing}, {tooClose, landed}}, std::nullopt, facts);

    behavior.decide(
        0.01f,
        told(facts, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(150.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "flee");

    facts["inReach"] = true;
    InputIntentions leap = behavior.decide(
        0.01f,
        told(facts, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(176.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "pounce");
    REQUIRE(leap.attack == PounceAttack);
    REQUIRE(leap.direction.x == -1.0f);

    facts["inReach"] = false;
    for (int step = 0; step < 10; ++step)
        behavior.decide(
            0.01f,
            told(facts, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(176.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("A state that does nothing stands still", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData idling;
    idling.name = "idle";
    StateMachineBehavior behavior(StateMachineBehaviorData{{idling}, {}});

    InputIntentions standing = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(384.0f, 192.0f)));

    REQUIRE(behavior.getStateName() == "idle");
    REQUIRE(standing.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getCurrentNodeId().has_value());
}

TEST_CASE("A transition asking about a fact nobody declares is refused", "[StateMachineBehavior]")
{
    StateMachineBehaviorData data = setupData();
    data.transitions.front().when["snowing"] = true;

    REQUIRE_THROWS_AS(StateMachineBehavior(data, std::nullopt, knowingNear()), std::runtime_error);

    REQUIRE_THROWS_AS(StateMachineBehavior(setupData()), std::runtime_error);
}

TEST_CASE("A transition asking with the wrong kind of value is refused", "[StateMachineBehavior]")
{
    StateMachineBehaviorData data = setupData();
    data.transitions.front().when["near"] = 3.0f;
    REQUIRE_THROWS_AS(StateMachineBehavior(data, std::nullopt, knowingNear()), std::runtime_error);

    data = setupData();
    data.transitions.front().when["onGround"] = 3.0f;
    REQUIRE_THROWS_AS(StateMachineBehavior(data, std::nullopt, knowingNear()), std::runtime_error);
}

TEST_CASE("A transition asking about a fact nobody has said is an error", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(), std::nullopt, knowingNear());
    Facts nothing;

    REQUIRE_THROWS_AS(
        behavior.decide(0.01f, told(nothing, standingAt(navigationGraph, {192.0f, 192.0f}))),
        std::runtime_error);
    REQUIRE_THROWS_AS(
        behavior.decide(0.01f, standingAt(navigationGraph, {192.0f, 192.0f})), std::runtime_error);
}

TEST_CASE(
    "A transition can wait for a charge to end, which the engine answers",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData charging;
    charging.name = "charge";
    BehaviorStateData stunned;
    stunned.name = "stunned";
    BehaviorTransitionData spent;
    spent.from = "charge";
    spent.to = "stunned";
    spent.when["charging"] = false;
    StateMachineBehavior behavior(StateMachineBehaviorData{{charging, stunned}, {spent}});
    Decided decided;
    decided.charge.active = true;
    ActorBehaviorContext midCharge = standingAt(navigationGraph, {96.0f, 192.0f});
    midCharge.decided = &decided;

    behavior.decide(0.01f, midCharge);
    REQUIRE(behavior.getStateName() == "charge");

    decided.charge.active = false;
    behavior.decide(0.01f, midCharge);
    REQUIRE(behavior.getStateName() == "stunned");
}

TEST_CASE("A machine told nothing of its abilities is not charging", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData charging;
    charging.name = "charge";
    BehaviorStateData stunned;
    stunned.name = "stunned";
    BehaviorTransitionData spent;
    spent.from = "charge";
    spent.to = "stunned";
    spent.when["charging"] = false;
    StateMachineBehavior behavior(StateMachineBehaviorData{{charging, stunned}, {spent}});

    behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}));

    REQUIRE(behavior.getStateName() == "stunned");
}
