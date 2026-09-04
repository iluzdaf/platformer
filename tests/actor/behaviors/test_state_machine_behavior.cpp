#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "actor/actor_behavior_context.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "navigation/navigation_edge.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    NavigationGraph setupRun(int nodeCount = 5, float spacing = 96.0f)
    {
        NavigationGraph navigationGraph;
        for (int id = 0; id < nodeCount; ++id)
            navigationGraph.addNode(id, {id * spacing, 192.0f});

        for (int id = 1; id < nodeCount; ++id)
        {
            navigationGraph.addEdge(id - 1, id, EdgeType::Walk);
            navigationGraph.addEdge(id, id - 1, EdgeType::Walk);
        }
        return navigationGraph;
    }

    ActorContactState standing()
    {
        ActorContactState contacts;
        contacts.onGround = true;
        return contacts;
    }

    ActorBehaviorContext at(
        const NavigationGraph &navigationGraph,
        glm::vec2 worldPosition,
        std::optional<glm::vec2> threatPosition)
    {
        return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), threatPosition, standing()};
    }

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
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(setupData());

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Stays alarmed while the threat is only a little further off", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(setupData());

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(130.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Calms down once the threat is well clear", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(setupData());

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Waits out the calm down before going back to patrol", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(setupData(48.0f, 1.0f));

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 50; ++step)
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 60; ++step)
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("A threat that comes back resets the calm down", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(setupData(48.0f, 1.0f));

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    for (int step = 0; step < 90; ++step)
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));

    for (int step = 0; step < 90; ++step)
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    for (int step = 0; step < 20; ++step)
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Runs the state it is in, not the one it left", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(setupData());

    InputIntentions patrolling =
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(patrolling.direction.x == -1.0f);

    InputIntentions fleeing =
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(fleeing.direction.x == 1.0f);
}

TEST_CASE("Given no states at all it asks for nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(StateMachineBehaviorData{});

    InputIntentions inputIntentions =
        behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 0.0f);
}

TEST_CASE("Holds a state while the threat shares its run", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    navigationGraph.addNode(5, {192.0f, 288.0f});

    StateMachineBehaviorData data = setupData();
    data.transitions.at(1) =
        BehaviorTransitionData{"flee", "patrol", std::nullopt, std::nullopt, false, 0.0f};

    StateMachineBehavior behavior(data);

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(192.0f, 288.0f)));
    REQUIRE(behavior.getStateName() == "patrol");
}

TEST_CASE("Ignores a threat that is close by but not on its ground", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    navigationGraph.addNode(5, {192.0f, 224.0f});

    StateMachineBehaviorData data = setupData();
    data.transitions.at(0) =
        BehaviorTransitionData{"patrol", "flee", 48.0f, std::nullopt, true, 0.0f};

    StateMachineBehavior behavior(data);

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(192.0f, 224.0f)));
    REQUIRE(behavior.getStateName() == "patrol");

    behavior.decide(0.01f, at(navigationGraph, {192.0f, 192.0f}, glm::vec2(160.0f, 192.0f)));
    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("A state with nothing to do asks for nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    BehaviorStateData idle;
    idle.name = "idle";
    StateMachineBehaviorData data;
    data.states = {idle};
    StateMachineBehavior behavior(data, std::nullopt);

    InputIntentions asked =
        behavior.decide(0.016f, at(navigationGraph, {0.0f, 192.0f}, std::nullopt));

    REQUIRE(behavior.getStateName() == "idle");
    REQUIRE(asked.direction.x == 0.0f);
    REQUIRE_FALSE(asked.jumpRequested);
}

TEST_CASE("A transition to a state it does not have is ignored", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehaviorData data = setupData();
    BehaviorTransitionData haunted;
    haunted.from = "flee";
    haunted.to = "ghost";
    haunted.threatWithin = 48.0f;
    data.transitions.push_back(haunted);
    StateMachineBehavior behavior(data, std::nullopt);
    ActorBehaviorContext threatened = at(navigationGraph, {0.0f, 192.0f}, glm::vec2(8.0f, 192.0f));

    behavior.decide(0.016f, threatened);
    REQUIRE(behavior.getStateName() == "flee");

    behavior.decide(0.016f, threatened);

    REQUIRE(behavior.getStateName() == "flee");
}

TEST_CASE("Given no states at all, resetting is nothing", "[StateMachineBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    StateMachineBehavior behavior(StateMachineBehaviorData{}, std::nullopt);

    behavior.reset();

    REQUIRE(
        behavior.decide(0.016f, at(navigationGraph, {0.0f, 192.0f}, std::nullopt)).direction.x ==
        0.0f);
}
