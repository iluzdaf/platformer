#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <optional>
#include "actor/actor_behavior_context.hpp"
#include "helpers/behaviour_context.hpp"
#include "actor/behaviors/chase_behavior.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "input/input_intentions.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    constexpr float Reach = 8.0f * 0.5f + 2.0f;

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

    ChaseBehaviorData setupData()
    {
        ChaseBehaviorData data;
        data.arrivalThreshold = 2.0f;
        return data;
    }

    glm::vec2 closeIn(
        ChaseBehavior &behavior,
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
        return position;
    }

    bool beside(glm::vec2 position, glm::vec2 threat)
    {
        return std::abs(position.x - threat.x) <= Reach;
    }
}

TEST_CASE("Closes on a threat standing further along its run", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());
    glm::vec2 threat(384.0f, 192.0f);

    glm::vec2 endedAt = closeIn(behavior, navigationGraph, {96.0f, 192.0f}, threat);

    REQUIRE(beside(endedAt, threat));
    REQUIRE(behavior.getCurrentNodeId() == 3);
}

TEST_CASE("Comes back the other way when the threat is behind it", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());
    glm::vec2 threat(0.0f, 192.0f);

    REQUIRE(beside(closeIn(behavior, navigationGraph, {288.0f, 192.0f}, threat), threat));
}

TEST_CASE("Stops beside the threat instead of running through it", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());
    glm::vec2 threat(200.0f, 192.0f);

    glm::vec2 endedAt = closeIn(behavior, navigationGraph, {96.0f, 192.0f}, threat);
    InputIntentions holding = behavior.decide(0.01f, standingAt(navigationGraph, endedAt, threat));

    REQUIRE(beside(endedAt, threat));
    REQUIRE(holding.direction.x == 0.0f);
}

TEST_CASE("Follows a threat that moves on", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());

    glm::vec2 caughtAt =
        closeIn(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(200.0f, 192.0f));
    REQUIRE(beside(caughtAt, glm::vec2(200.0f, 192.0f)));

    glm::vec2 movedOn(320.0f, 192.0f);
    InputIntentions setsOff =
        behavior.decide(0.01f, standingAt(navigationGraph, caughtAt, movedOn));
    REQUIRE(setsOff.direction.x == 1.0f);

    REQUIRE(beside(closeIn(behavior, navigationGraph, caughtAt, movedOn), movedOn));
}

TEST_CASE("Turns the moment the threat doubles back behind it", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());

    glm::vec2 midway =
        closeIn(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(384.0f, 192.0f), 40);
    REQUIRE(midway.x > 96.0f);
    REQUIRE(midway.x < 288.0f);

    InputIntentions turning =
        behavior.decide(0.01f, standingAt(navigationGraph, midway, glm::vec2(0.0f, 192.0f)));

    REQUIRE(turning.direction.x == -1.0f);
}

TEST_CASE("Stands still while there is nothing to chase", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());

    InputIntentions inputIntentions =
        behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, std::nullopt));

    REQUIRE(inputIntentions.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getTargetNodeId().has_value());
}

TEST_CASE("Sets off again once a threat appears", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());

    behavior.decide(0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, std::nullopt));
    InputIntentions inputIntentions = behavior.decide(
        0.01f, standingAt(navigationGraph, {96.0f, 192.0f}, glm::vec2(384.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 1.0f);
}

TEST_CASE("Will not chase somewhere it cannot get back from", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = setupRunWithAOneWayCrossing();
    ChaseBehavior behavior(setupData());

    glm::vec2 endedAt =
        closeIn(behavior, navigationGraph, {96.0f, 192.0f}, glm::vec2(576.0f, 192.0f));

    REQUIRE(behavior.getCurrentNodeId() == 4);
    REQUIRE(std::abs(endedAt.x - 384.0f) <= Reach);
}

TEST_CASE("Waits below a threat it has no way up to", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    navigationGraph.addNode(5, {192.0f, 96.0f});
    ChaseBehavior behavior(setupData());
    glm::vec2 threat(192.0f, 96.0f);

    glm::vec2 endedAt = closeIn(behavior, navigationGraph, {0.0f, 192.0f}, threat);
    InputIntentions waiting = behavior.decide(0.01f, standingAt(navigationGraph, endedAt, threat));

    REQUIRE(std::abs(endedAt.x - 192.0f) <= Reach);
    REQUIRE(waiting.direction.x == 0.0f);
}

TEST_CASE("Keeps going for a threat on a ledge it can jump to", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    navigationGraph.addNode(5, {192.0f, 96.0f});
    navigationGraph.addEdge({2, 5, EdgeType::Jump, {}, 0.2f});
    navigationGraph.addEdge(5, 2, EdgeType::Fall);
    ChaseBehavior behavior(setupData());
    glm::vec2 threat(192.0f, 96.0f);

    glm::vec2 endedAt = closeIn(behavior, navigationGraph, {0.0f, 192.0f}, threat);
    InputIntentions leaping = behavior.decide(0.01f, standingAt(navigationGraph, endedAt, threat));

    REQUIRE(std::abs(endedAt.x - 192.0f) <= Reach);
    REQUIRE(behavior.getTargetNodeId() == 5);
    REQUIRE(leaping.jumpRequested);
}

TEST_CASE("Has nothing to do on a graph with no edges at all", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 192.0f});
    ChaseBehavior behavior(setupData());

    InputIntentions inputIntentions = behavior.decide(
        0.01f, standingAt(navigationGraph, {0.0f, 192.0f}, glm::vec2(96.0f, 192.0f)));

    REQUIRE(inputIntentions.direction.x == 0.0f);
    REQUIRE_FALSE(behavior.getCurrentNodeId().has_value());
}

TEST_CASE("Catches up with feet settled a pixel below the run", "[ChaseBehavior]")
{
    NavigationGraph navigationGraph = aWalkRun();
    ChaseBehavior behavior(setupData());
    glm::vec2 threat(200.0f, 192.0f);

    glm::vec2 position(96.0f, 193.5f);
    for (int step = 0; step < 400; ++step)
    {
        InputIntentions inputIntentions =
            behavior.decide(0.01f, standingAt(navigationGraph, position, threat));
        position.x += inputIntentions.direction.x * 2.0f;
    }
    InputIntentions holding = behavior.decide(0.01f, standingAt(navigationGraph, position, threat));

    REQUIRE(beside(position, threat));
    REQUIRE(holding.direction.x == 0.0f);
}
