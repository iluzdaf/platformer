#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "actor/actor_behavior_context.hpp"
#include "actor/actor_contact_state.hpp"
#include "actor/behaviors/route_walker.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    constexpr float ArrivalThreshold = 2.0f;

    NavigationGraph setupPlatform()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 192.0f});
        navigationGraph.addNode(1, {96.0f, 192.0f});
        navigationGraph.addEdge(0, 1, EdgeType::Walk);
        navigationGraph.addEdge(1, 0, EdgeType::Walk);
        return navigationGraph;
    }

    ActorBehaviorContext at(const NavigationGraph &navigationGraph, glm::vec2 worldPosition)
    {
        ActorContactState standing;
        standing.onGround = true;
        return {navigationGraph, worldPosition, glm::vec2(8.0f, 13.0f), std::nullopt, standing};
    }
}

TEST_CASE("A walker with no footing takes no route", "[RouteWalker]")
{
    NavigationGraph navigationGraph = setupPlatform();
    RouteWalker walker(ArrivalThreshold);

    walker.takeRouteTo(at(navigationGraph, {0.0f, 192.0f}), 1);

    REQUIRE(walker.routeFinished());
    REQUIRE_FALSE(walker.getTargetNodeId());
}

TEST_CASE("A walker takes no route to somewhere it cannot reach", "[RouteWalker]")
{
    NavigationGraph navigationGraph = setupPlatform();
    navigationGraph.addNode(2, {300.0f, 192.0f});
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {0.0f, 192.0f}));
    REQUIRE(walker.getCurrentNodeId() == 0);

    walker.takeRouteTo(at(navigationGraph, {0.0f, 192.0f}), 2);

    REQUIRE(walker.routeFinished());
}

TEST_CASE("A walker above its node has not reached it", "[RouteWalker]")
{
    NavigationGraph navigationGraph = setupPlatform();
    RouteWalker walker(ArrivalThreshold);
    ActorBehaviorContext hangingAbove = at(navigationGraph, {0.0f, 150.0f});
    walker.keepInStep(hangingAbove);
    REQUIRE(walker.getCurrentNodeId() == 0);

    walker.takeRouteTo(hangingAbove, 0);

    REQUIRE(walker.getTargetNodeId() == 0);
}
