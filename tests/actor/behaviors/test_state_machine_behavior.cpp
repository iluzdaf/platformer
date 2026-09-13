#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include <optional>
#include "actor/actor_facts.hpp"
#include "helpers/actor_facts.hpp"
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_script.hpp"
#include "actor/behaviors/route_walker.hpp"
#include <vector>
#include "actor/behaviors/senses_data.hpp"
#include <catch2/matchers/catch_matchers_string.hpp>
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"
#include "state_machines/state_machine_data.hpp"

namespace
{
    StateMachineBehaviorData setupData(float calmDown = 0.0f)
    {
        BehaviorStateData watching;
        watching.name = "watch";
        watching.does = IdleBehaviorData{};

        BehaviorStateData chasing;
        chasing.name = "chase";
        chasing.does = ChaseBehaviorData{};

        TransitionData alarmed;
        alarmed.from = "watch";
        alarmed.to = "chase";
        alarmed.when["near"] = true;

        TransitionData calmed;
        calmed.from = "chase";
        calmed.to = "watch";
        calmed.when["near"] = false;
        calmed.after = calmDown;

        return {{watching, chasing}, {alarmed, calmed}};
    }

    FactsData knowingNear()
    {
        FactsData facts;
        facts["near"] = false;
        return facts;
    }

    FactsData saying(const std::string &name, const Asked &value)
    {
        FactsData facts = knowingNear();
        facts[name] = value;
        return facts;
    }

    ActorFacts told(const FactsData &facts, ActorFacts context)
    {
        context.facts = &facts;
        return context;
    }
}

TEST_CASE("Runs the state it is in, not the one it left", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(), knowingNear());
    FactsData calm = knowingNear();
    FactsData near = saying("near", true);

    InputIntentions watching = behavior.decide(
        0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f))));
    REQUIRE(watching.direction.x == 0.0f);

    InputIntentions chasing = behavior.decide(
        0.01f,
        told(near, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(300.0f, 192.0f))));
    REQUIRE(chasing.direction.x == 1.0f);
}

TEST_CASE("Given no states at all it asks for nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(StateMachineBehaviorData{});

    InputIntentions inputIntentions = behavior.decide(
        0.01f, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 0.0f);
}

TEST_CASE("A state with nothing to do asks for nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData idle;
    idle.name = "idle";
    StateMachineBehaviorData data;
    data.states = {idle};
    StateMachineBehavior behavior(data);

    InputIntentions asked =
        behavior.decide(0.016f, standingAt(navigationGraph, {0.0f, 192.0f}, std::nullopt));

    REQUIRE(behavior.getStateName() == "idle");
    REQUIRE(asked.direction.x == 0.0f);
    REQUIRE_FALSE(asked.jumpRequested);
}

TEST_CASE("Given no states at all, resetting is nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(StateMachineBehaviorData{});

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

        TransitionData close;
        close.from = "chase";
        close.to = "pounce";
        close.when["near"] = true;

        TransitionData landed;
        landed.from = "pounce";
        landed.to = "chase";
        landed.when["onGround"] = true;
        landed.after = 0.05f;

        return {{chasing, pouncing}, {close, landed}};
    }
}

TEST_CASE(
    "A transition can wait for the ground, which the engine answers",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aChaseThatPounces(1.0f), knowingNear());
    FactsData near = saying("near", true);
    behavior.decide(
        0.01f,
        told(near, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(200.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "pounce");

    ActorFacts inTheAir = told(near, airborneAt(navigationGraph, {196.0f, 180.0f}));
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
    "A chasing creature pounces when told the threat is in reach, then chases again once it lands",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData chasing;
    chasing.name = "chase";
    chasing.does = ChaseBehaviorData{};
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    pouncing.cooldown = 1.0f;
    TransitionData tooClose;
    tooClose.from = "chase";
    tooClose.to = "pounce";
    tooClose.when["inReach"] = true;
    TransitionData landed;
    landed.from = "pounce";
    landed.to = "chase";
    landed.when["onGround"] = true;
    landed.after = 0.05f;
    FactsData facts;
    facts["inReach"] = false;
    StateMachineBehavior behavior({{chasing, pouncing}, {tooClose, landed}}, facts);

    behavior.decide(
        0.01f,
        told(facts, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(150.0f, 192.0f))));
    REQUIRE(behavior.getStateName() == "chase");

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
    REQUIRE(behavior.getStateName() == "chase");
}

TEST_CASE("Each time it enters a state, that state starts afresh", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData chasing;
    chasing.name = "chase";
    chasing.does = ChaseBehaviorData{};
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    TransitionData tooClose;
    tooClose.from = "chase";
    tooClose.to = "pounce";
    tooClose.when["inReach"] = true;
    TransitionData backOff;
    backOff.from = "pounce";
    backOff.to = "chase";
    backOff.when["inReach"] = false;
    FactsData facts;
    facts["inReach"] = false;
    StateMachineBehavior behavior({{chasing, pouncing}, {tooClose, backOff}}, facts);
    auto decide = [&](bool inReach)
    {
        facts["inReach"] = inReach;
        return behavior.decide(
            0.01f,
            told(facts, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(176.0f, 192.0f))));
    };

    REQUIRE(decide(true).attack == PounceAttack);
    REQUIRE(decide(true).attack.empty());
    decide(false);
    REQUIRE(behavior.getStateName() == "chase");

    REQUIRE(decide(true).attack == PounceAttack);
}

TEST_CASE("Resetting goes back to the first state, and starts it afresh", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    BehaviorStateData chasing;
    chasing.name = "chase";
    chasing.does = ChaseBehaviorData{};
    TransitionData backOff;
    backOff.from = "pounce";
    backOff.to = "chase";
    backOff.when["inReach"] = false;
    FactsData facts;
    facts["inReach"] = true;
    StateMachineBehavior behavior({{pouncing, chasing}, {backOff}}, facts);
    auto decide = [&](bool inReach)
    {
        facts["inReach"] = inReach;
        return behavior.decide(
            0.01f,
            told(facts, standingAt(navigationGraph, {192.0f, 192.0f}, glm::vec2(176.0f, 192.0f))));
    };
    REQUIRE(decide(true).attack == PounceAttack);
    decide(false);
    REQUIRE(behavior.getStateName() == "chase");

    behavior.reset();

    REQUIRE(behavior.getStateName() == "pounce");
    REQUIRE(decide(true).attack == PounceAttack);
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

TEST_CASE("A transition asking about a fact nobody has said is an error", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(setupData(), knowingNear());
    FactsData nothing;

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
    TransitionData spent;
    spent.from = "charge";
    spent.to = "stunned";
    spent.when["charging"] = false;
    StateMachineBehavior behavior(StateMachineBehaviorData{{charging, stunned}, {spent}});
    AbilityStates states;
    states.charge.active = true;
    ActorFacts midCharge = standingAt(navigationGraph, {96.0f, 192.0f});
    midCharge.abilityStates = &states;

    behavior.decide(0.01f, midCharge);
    REQUIRE(behavior.getStateName() == "charge");

    states.charge.active = false;
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
    TransitionData spent;
    spent.from = "charge";
    spent.to = "stunned";
    spent.when["charging"] = false;
    StateMachineBehavior behavior(StateMachineBehaviorData{{charging, stunned}, {spent}});

    behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}));

    REQUIRE(behavior.getStateName() == "stunned");
}

TEST_CASE(
    "A transition asking how close the threat is needs a sense of it",
    "[StateMachineBehavior]")
{
    StateMachineBehaviorData data = setupData();
    data.transitions.front().when["threatClose"] = true;
    data.transitions.front().when["threatInReach"] = true;

    REQUIRE_THROWS_WITH(
        StateMachineBehavior(data, knowingNear()),
        Catch::Matchers::ContainsSubstring("its senses say nothing of close"));
    REQUIRE_THROWS_WITH(
        StateMachineBehavior(data, knowingNear(), SensesData{40.0f, std::nullopt}),
        Catch::Matchers::ContainsSubstring("its senses say nothing of reach"));
    REQUIRE_NOTHROW(StateMachineBehavior(data, knowingNear(), SensesData{40.0f, 24.0f}));
}

TEST_CASE(
    "A transition may ask what the actor is doing, as an animation can",
    "[StateMachineBehavior]")
{
    StateMachineBehaviorData data = setupData();
    data.transitions.front().when.clear();
    data.transitions.front().when["knockback"] = true;
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(data, knowingNear());
    FactsData calm = knowingNear();
    AbilityStates states;
    ActorFacts facts = told(calm, standingAt(navigationGraph, {192.0f, 192.0f}));
    facts.abilityStates = &states;

    behavior.decide(0.01f, facts);
    REQUIRE(behavior.getStateName() == "watch");

    states.knockback.active = true;
    behavior.decide(0.01f, facts);
    REQUIRE(behavior.getStateName() == "chase");
}

TEST_CASE("A transition cannot ask about the picture", "[StateMachineBehavior]")
{
    for (const std::string &picture : {std::string("finished"), std::string("inState")})
    {
        StateMachineBehaviorData data = setupData();
        data.transitions.front().when[picture] = true;

        INFO(picture);
        REQUIRE_THROWS_WITH(
            StateMachineBehavior(data, knowingNear()),
            Catch::Matchers::ContainsSubstring("there is no such fact"));
    }
}

namespace
{
    struct Hearing : StateScript
    {
        std::vector<std::string> heard;

        void enter(const std::string &call) override
        {
            heard.push_back("enter " + call);
        }

        InputIntentions decide(const std::string &call, RouteWalker &, const ActorFacts &, float)
            override
        {
            heard.push_back("decide " + call);
            return {};
        }

        void exit(const std::string &call) override
        {
            heard.push_back("exit " + call);
        }
    };

    StateMachineBehaviorData aWatchThatRuns()
    {
        BehaviorStateData watching;
        watching.name = "watch";
        watching.does = ScriptedBehaviorData{"watch"};
        BehaviorStateData running;
        running.name = "run";
        running.does = ScriptedBehaviorData{"run"};
        TransitionData startled;
        startled.from = "watch";
        startled.to = "run";
        startled.when["near"] = true;
        return {{watching, running}, {startled}};
    }
}

TEST_CASE(
    "A scripted state hears it is left, and the one entered starts afresh",
    "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aWatchThatRuns(), knowingNear());
    Hearing script;
    behavior.scriptWith(&script);
    FactsData calm = knowingNear();
    FactsData near = saying("near", true);

    behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));
    behavior.decide(0.01f, told(near, standingAt(navigationGraph, {192.0f, 192.0f})));

    REQUIRE(
        script.heard ==
        std::vector<std::string>{
            "enter watch", "decide watch", "exit watch", "enter run", "decide run"});
}

TEST_CASE("Resetting leaves the state it is in", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    StateMachineBehavior behavior(aWatchThatRuns(), knowingNear());
    Hearing script;
    behavior.scriptWith(&script);
    FactsData calm = knowingNear();
    behavior.decide(0.01f, told(calm, standingAt(navigationGraph, {192.0f, 192.0f})));

    behavior.reset();

    REQUIRE(script.heard.back() == "exit watch");
}
