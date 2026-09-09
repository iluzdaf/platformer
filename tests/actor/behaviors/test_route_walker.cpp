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

    ActorBehaviorContext at(const NavigationGraph &navigationGraph, glm::vec2 feet)
    {
        ActorContactState standing;
        standing.onGround = true;
        return {navigationGraph, feet, glm::vec2(8.0f, 13.0f), std::nullopt, standing};
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

namespace
{
    constexpr float Upper = 160.0f;
    constexpr float Lower = 192.0f;

    NavigationGraph twoRunsOneAboveTheOther()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {192.0f, Upper});
        navigationGraph.addNode(1, {304.0f, Upper});
        navigationGraph.addNode(2, {176.0f, Lower});
        navigationGraph.addNode(3, {304.0f, Lower});
        navigationGraph.addEdge(0, 1, EdgeType::Walk);
        navigationGraph.addEdge(1, 0, EdgeType::Walk);
        navigationGraph.addEdge(2, 3, EdgeType::Walk);
        navigationGraph.addEdge(3, 2, EdgeType::Walk);
        navigationGraph.addEdge(0, 2, EdgeType::Fall);
        navigationGraph.addEdge(2, 0, EdgeType::Jump);
        return navigationGraph;
    }

    bool onTheUpperRun(std::optional<int> nodeId)
    {
        return nodeId == 0 || nodeId == 1;
    }
}

TEST_CASE("Feet settled just below a run anchor to that run, not the one beneath", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);

    walker.keepInStep(at(navigationGraph, {250.0f, Upper + 1.5f}));

    INFO("anchored to node " << walker.getCurrentNodeId().value_or(-1));
    REQUIRE(onTheUpperRun(walker.getCurrentNodeId()));
}

TEST_CASE("A route is not lost to feet settling a pixel or two below the run", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {250.0f, Upper}));
    REQUIRE(onTheUpperRun(walker.getCurrentNodeId()));
    walker.takeRouteTo(at(navigationGraph, {250.0f, Upper}), 0);
    REQUIRE(walker.getTargetNodeId() == 0);

    walker.keepInStep(at(navigationGraph, {250.0f, Upper + 1.5f}));

    REQUIRE(onTheUpperRun(walker.getCurrentNodeId()));
    REQUIRE(walker.getTargetNodeId() == 0);
}

TEST_CASE("Feet a whole drop below a run have left it", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {250.0f, Upper}));
    walker.takeRouteTo(at(navigationGraph, {250.0f, Upper}), 0);

    walker.keepInStep(at(navigationGraph, {250.0f, Lower}));

    REQUIRE_FALSE(onTheUpperRun(walker.getCurrentNodeId()));
    REQUIRE_FALSE(walker.getTargetNodeId().has_value());
}
