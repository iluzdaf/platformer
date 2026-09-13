#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "actor/actor_facts.hpp"
#include "helpers/actor_facts.hpp"
#include "helpers/shipped_steering.hpp"
#include "navigation/navigation_edge.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    NavigationGraph setupRunWithAOneWayCrossing()
    {
        NavigationGraph navigationGraph = aWalkRun();
        navigationGraph.addNode(5, {480.0f, 192.0f});
        navigationGraph.addNode(6, {576.0f, 192.0f});
        navigationGraph.addEdge(5, 6, EdgeType::Walk);
        navigationGraph.addEdge(6, 5, EdgeType::Walk);

        navigationGraph.addEdge(4, 5, EdgeType::Walk);
        return navigationGraph;
    }

    int runAway(
        ShippedSteering &behavior,
        const NavigationGraph &navigationGraph,
        glm::vec2 start,
        std::optional<glm::vec2> threatFeet,
        int steps = 400)
    {
        glm::vec2 position = start;
        for (int step = 0; step < steps; ++step)
        {
            InputIntentions inputIntentions =
                behavior.decide(0.01f, standingAt(navigationGraph, position, threatFeet));
            position.x += inputIntentions.direction.x * 2.0f;
        }
        return *behavior.getCurrentNodeId();
    }
}

TEST_CASE("Runs to the far end of the run it is on", "[RatFlees]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ShippedSteering behavior("rat", "flee");

    REQUIRE(runAway(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(0.0f, 192.0f)) == 4);
}

TEST_CASE("Runs the other way when the threat comes from the other side", "[RatFlees]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ShippedSteering behavior("rat", "flee");

    REQUIRE(runAway(behavior, navigationGraph, {288.0f, 192.0f}, glm::vec2(384.0f, 192.0f)) == 0);
}

TEST_CASE("Turns round when the threat gets between it and its refuge", "[RatFlees]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ShippedSteering behavior("rat", "flee");

    glm::vec2 position(96.0f, 192.0f);
    for (int step = 0; step < 40; ++step)
    {
        InputIntentions inputIntentions =
            behavior.decide(0.01f, standingAt(navigationGraph, position, glm::vec2(0.0f, 192.0f)));
        position.x += inputIntentions.direction.x * 2.0f;
    }

    REQUIRE(position.x > 96.0f);

    InputIntentions cornered =
        behavior.decide(0.01f, standingAt(navigationGraph, position, glm::vec2(384.0f, 192.0f)));

    REQUIRE(cornered.direction.x == -1.0f);
}

TEST_CASE("Will not escape somewhere it cannot get back from", "[RatFlees]")
{
    NavigationGraph navigationGraph = setupRunWithAOneWayCrossing();
    ShippedSteering behavior("rat", "flee");

    int endedAt = runAway(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(0.0f, 192.0f));

    REQUIRE(endedAt == 4);
}

TEST_CASE("Stands still while nothing is chasing it", "[RatFlees]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ShippedSteering behavior("rat", "flee");

    InputIntentions inputIntentions =
        behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, std::nullopt));

    REQUIRE(inputIntentions.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getTargetNodeId().has_value());
    REQUIRE(behavior.errorsReported() == 0);
}

TEST_CASE("Keeps to its route while it is in the air", "[RatFlees]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ShippedSteering behavior("rat", "flee");
    glm::vec2 position(96.0f, 192.0f);
    for (int step = 0; step < 40; ++step)
    {
        InputIntentions running =
            behavior.decide(0.01f, standingAt(navigationGraph, position, glm::vec2(0.0f, 192.0f)));
        position.x += running.direction.x * 2.0f;
    }
    REQUIRE(position.x > 96.0f);

    ActorFacts leaping = airborneAt(navigationGraph, position);
    leaping.threatFeet = glm::vec2(384.0f, 192.0f);

    REQUIRE(behavior.decide(0.01f, leaping).direction.x == 1.0f);
    REQUIRE(behavior.errorsReported() == 0);
}

TEST_CASE("Has nothing to do on a graph with no edges at all", "[RatFlees]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 192.0f});
    ShippedSteering behavior("rat", "flee");

    InputIntentions inputIntentions = behavior.decide(
        0.01f, standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(0.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getCurrentNodeId().has_value());
}

TEST_CASE("Will not run past the threat to reach open ground", "[RatFlees]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ShippedSteering behavior("rat", "flee");

    REQUIRE(runAway(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(144.0f, 192.0f)) == 0);
}

TEST_CASE("Turns back to the node it set off from when the threat gets behind it", "[RatFlees]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    ShippedSteering behavior("rat", "flee");

    glm::vec2 position(104.0f, 96.0f);
    for (int step = 0; step < 40; ++step)
    {
        InputIntentions running =
            behavior.decide(0.01f, standingAt(navigationGraph, position, glm::vec2(112.0f, 96.0f)));
        position.x += running.direction.x * 1.0f;
    }

    REQUIRE(position.x < 80.0f);
    REQUIRE(behavior.getCurrentNodeId() == 1);

    InputIntentions turningBack =
        behavior.decide(0.01f, standingAt(navigationGraph, position, glm::vec2(16.0f, 96.0f)));

    REQUIRE(turningBack.direction.x == 1.0f);
}

TEST_CASE("Holds its corner however close the threat comes", "[RatFlees]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    ShippedSteering behavior("rat", "flee");

    glm::vec2 position(104.0f, 96.0f);
    for (int step = 0; step < 200; ++step)
    {
        InputIntentions running =
            behavior.decide(0.01f, standingAt(navigationGraph, position, glm::vec2(112.0f, 96.0f)));
        position.x += running.direction.x * 1.0f;
    }
    REQUIRE(position.x < 24.0f);

    InputIntentions holding = behavior.decide(
        0.01f, standingAt(navigationGraph, position, glm::vec2(position.x + 10.0f, 96.0f)));

    REQUIRE(holding.direction.x == 0.0f);
}
