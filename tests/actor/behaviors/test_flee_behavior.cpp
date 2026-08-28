#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "actor/behaviors/flee_behavior.hpp"
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

    NavigationGraph setupRunWithAOneWayCrossing()
    {
        NavigationGraph navigationGraph = setupRun();
        navigationGraph.addNode(5, {480.0f, 192.0f});
        navigationGraph.addNode(6, {576.0f, 192.0f});
        navigationGraph.addEdge(5, 6, EdgeType::Walk);
        navigationGraph.addEdge(6, 5, EdgeType::Walk);

        navigationGraph.addEdge(4, 5, EdgeType::Walk);
        return navigationGraph;
    }

    ActorBehaviorContext at(
        const NavigationGraph &navigationGraph,
        glm::vec2 worldPosition,
        std::optional<glm::vec2> threatPosition)
    {
        return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), threatPosition, true};
    }

    FleeBehaviorData setupData()
    {
        FleeBehaviorData data;
        data.arrivalThreshold = 2.0f;
        return data;
    }

    int runAway(
        FleeBehavior &behavior,
        const NavigationGraph &navigationGraph,
        glm::vec2 start,
        std::optional<glm::vec2> threatPosition,
        int steps = 400)
    {
        glm::vec2 position = start;
        for (int step = 0; step < steps; ++step)
        {
            InputIntentions inputIntentions =
                behavior.decide(0.01f, at(navigationGraph, position, threatPosition));
            position.x += inputIntentions.direction.x * 2.0f;
        }
        return *behavior.getCurrentNodeId();
    }
}

TEST_CASE("Runs to the far end of the run it is on", "[FleeBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    FleeBehavior behavior(setupData());

    REQUIRE(runAway(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(0.0f, 192.0f)) == 4);
}

TEST_CASE("Runs the other way when the threat comes from the other side", "[FleeBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    FleeBehavior behavior(setupData());

    REQUIRE(runAway(behavior, navigationGraph, {288.0f, 192.0f}, glm::vec2(384.0f, 192.0f)) == 0);
}

TEST_CASE("Turns round when the threat gets between it and its refuge", "[FleeBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    FleeBehavior behavior(setupData());

    glm::vec2 position(96.0f, 192.0f);
    for (int step = 0; step < 40; ++step)
    {
        InputIntentions inputIntentions =
            behavior.decide(0.01f, at(navigationGraph, position, glm::vec2(0.0f, 192.0f)));
        position.x += inputIntentions.direction.x * 2.0f;
    }

    REQUIRE(position.x > 96.0f);

    InputIntentions cornered =
        behavior.decide(0.01f, at(navigationGraph, position, glm::vec2(384.0f, 192.0f)));

    REQUIRE(cornered.direction.x == -1.0f);
}

TEST_CASE("Will not escape somewhere it cannot get back from", "[FleeBehavior]")
{
    NavigationGraph navigationGraph = setupRunWithAOneWayCrossing();
    FleeBehavior behavior(setupData());

    int endedAt = runAway(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(0.0f, 192.0f));

    REQUIRE(endedAt == 4);
}

TEST_CASE("Stands still while nothing is chasing it", "[FleeBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    FleeBehavior behavior(setupData());

    InputIntentions inputIntentions =
        behavior.decide(0.01f, at(navigationGraph, {96.0f, 192.0f}, std::nullopt));

    REQUIRE(inputIntentions.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getTargetNodeId().has_value());
}

TEST_CASE("Has nothing to do on a graph with no edges at all", "[FleeBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 192.0f});
    FleeBehavior behavior(setupData());

    InputIntentions inputIntentions =
        behavior.decide(0.01f, at(navigationGraph, {0.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getCurrentNodeId().has_value());
}

TEST_CASE("Will not run past the threat to reach open ground", "[FleeBehavior]")
{
    NavigationGraph navigationGraph = setupRun();
    FleeBehavior behavior(setupData());

    REQUIRE(runAway(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(144.0f, 192.0f)) == 0);
}
