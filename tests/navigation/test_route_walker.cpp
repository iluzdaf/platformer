#include <catch2/catch_test_macros.hpp>
#include <optional>
#include "actor/actor_facts.hpp"
#include "actor/actor_contact_state.hpp"
#include "navigation/route_walker.hpp"
#include "navigation/navigation_edge.hpp"
#include "input/input_intentions.hpp"
#include "navigation/footing.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"

namespace
{
    constexpr float ArrivalThreshold = 2.0f;
    constexpr float StepHeight = 3.0f;
    constexpr float AStepDown = StepHeight + 0.01f;

    NavigationGraph setupPlatform()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, {0.0f, 192.0f});
        navigationGraph.addNode(1, {96.0f, 192.0f});
        navigationGraph.addEdge(0, 1, EdgeType::Walk);
        navigationGraph.addEdge(1, 0, EdgeType::Walk);
        return navigationGraph;
    }

    ActorFacts at(const NavigationGraph &navigationGraph, glm::vec2 feet)
    {
        ActorContactState standing;
        standing.onGround = true;
        return {navigationGraph, feet, glm::vec2(8.0f, 13.0f), StepHeight, std::nullopt, standing};
    }
}

TEST_CASE("A walker told nothing has arrived once within 2 of its reach", "[RouteWalker]")
{
    NavigationGraph navigationGraph = setupPlatform();
    RouteWalker walker;
    walker.keepInStep(at(navigationGraph, {0.0f, 192.0f}));
    walker.takeRouteTo(at(navigationGraph, {0.0f, 192.0f}), 1);

    walker.advanceOnArrival(at(navigationGraph, {89.5f, 192.0f}));
    REQUIRE(walker.getCurrentNodeId() == 0);

    walker.advanceOnArrival(at(navigationGraph, {90.5f, 192.0f}));
    REQUIRE(walker.getCurrentNodeId() == 1);
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
    ActorFacts hangingAbove = at(navigationGraph, {0.0f, 150.0f});
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

TEST_CASE("Feet a step below a run anchor to that run, not the one beneath", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);

    walker.keepInStep(at(navigationGraph, {250.0f, Upper + AStepDown}));

    INFO("anchored to node " << walker.getCurrentNodeId().value_or(-1));
    REQUIRE(onTheUpperRun(walker.getCurrentNodeId()));
}

TEST_CASE("Feet on a run anchor to it, not to a run a pixel higher elsewhere", "[RouteWalker]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, Upper});
    navigationGraph.addNode(1, {45.0f, Upper + 1.0f});
    navigationGraph.addNode(2, {80.0f, Upper + 1.0f});
    navigationGraph.addNode(3, {160.0f, Upper + 1.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addEdge(2, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 2, EdgeType::Walk);
    RouteWalker walker(ArrivalThreshold);

    walker.keepInStep(at(navigationGraph, {80.0f, Upper + 1.0f}));

    REQUIRE(walker.getCurrentNodeId() == 2);
}

TEST_CASE("A route is not lost to feet a step below the run", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {250.0f, Upper}));
    REQUIRE(onTheUpperRun(walker.getCurrentNodeId()));
    walker.takeRouteTo(at(navigationGraph, {250.0f, Upper}), 0);
    REQUIRE(walker.getTargetNodeId() == 0);

    walker.keepInStep(at(navigationGraph, {250.0f, Upper + AStepDown}));

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

TEST_CASE("Feet more than a step below a run have left it", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {250.0f, Upper}));
    walker.takeRouteTo(at(navigationGraph, {250.0f, Upper}), 0);

    walker.keepInStep(at(navigationGraph, {250.0f, Upper + StepHeight + 1.0f}));

    REQUIRE_FALSE(walker.getTargetNodeId().has_value());
}

TEST_CASE("Feet more than a step above a run have left it", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {250.0f, Lower}));
    walker.takeRouteTo(at(navigationGraph, {250.0f, Lower}), 2);
    REQUIRE(walker.getTargetNodeId() == 2);

    walker.keepInStep(at(navigationGraph, {250.0f, Lower - StepHeight - 1.0f}));

    REQUIRE_FALSE(walker.getTargetNodeId().has_value());
}

TEST_CASE("A route is not lost along a run that steps down more than once", "[RouteWalker]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, Upper});
    navigationGraph.addNode(1, {96.0f, Upper + 2.0f * StepHeight});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {0.0f, Upper}));
    walker.takeRouteTo(at(navigationGraph, {0.0f, Upper}), 1);

    walker.keepInStep(at(navigationGraph, {80.0f, Upper + 2.0f * StepHeight}));

    REQUIRE(walker.getCurrentNodeId() == 0);
    REQUIRE(walker.getTargetNodeId() == 1);
}

TEST_CASE("A climber that stops just short of its node is there", "[RouteWalker]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {304.0f, Lower});
    navigationGraph.addNode(1, {304.0f, Upper}, NodeKind::OnWall);
    navigationGraph.addEdge(0, 1, EdgeType::Climb);
    navigationGraph.addEdge(1, 0, EdgeType::Climb);
    ActorContactState onTheWall;
    onTheWall.touchingRightWall = true;
    ActorFacts atTheFoot = at(navigationGraph, {304.0f, Lower});
    ActorFacts nearTheTop{
        navigationGraph,
        {304.0f, Upper + ClimbArrivesWithin * 0.9f},
        glm::vec2(8.0f, 13.0f),
        0.0f,
        std::nullopt,
        onTheWall};
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(atTheFoot);
    walker.takeRouteTo(atTheFoot, 1);

    walker.advanceOnArrival(nearTheTop);
    REQUIRE(walker.getCurrentNodeId() == 1);
    walker.takeRouteTo(nearTheTop, 1);

    REQUIRE(walker.routeFinished());
}

TEST_CASE("A walker arrives at its node with feet a step below it", "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {250.0f, Upper + AStepDown}));
    walker.takeRouteTo(at(navigationGraph, {250.0f, Upper + AStepDown}), 0);
    REQUIRE(walker.getTargetNodeId() == 0);

    walker.advanceOnArrival(at(navigationGraph, {192.0f, Upper + AStepDown}));

    REQUIRE(walker.routeFinished());
    REQUIRE(walker.getCurrentNodeId() == 0);
}

TEST_CASE(
    "A walker arrives at a stop short of the node with feet a step below the run",
    "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {200.0f, Upper + AStepDown}));
    walker.takeRouteTo(
        at(navigationGraph, {200.0f, Upper + AStepDown}), 1, glm::vec2(250.0f, Upper));

    walker.advanceOnArrival(at(navigationGraph, {250.0f, Upper + AStepDown}));

    REQUIRE(walker.routeFinished());
}

TEST_CASE(
    "A walker already at its destination with feet a step below it has nowhere to go",
    "[RouteWalker]")
{
    NavigationGraph navigationGraph = twoRunsOneAboveTheOther();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {192.0f, Upper + AStepDown}));
    REQUIRE(walker.getCurrentNodeId() == 0);

    walker.takeRouteTo(at(navigationGraph, {192.0f, Upper + AStepDown}), 0);

    REQUIRE(walker.routeFinished());
}

TEST_CASE("A walker whose ground is numbered afresh anchors again", "[RouteWalker]")
{
    NavigationGraph navigationGraph = setupPlatform();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {0.0f, 192.0f}));
    walker.takeRouteTo(at(navigationGraph, {0.0f, 192.0f}), 1);

    REQUIRE(walker.getCurrentNodeId() == 0);
    REQUIRE(walker.getTargetNodeId() == 1);

    NavigationGraph builtAgain;
    builtAgain.addNode(7, {0.0f, 192.0f});
    builtAgain.addNode(8, {96.0f, 192.0f});
    builtAgain.addEdge(7, 8, EdgeType::Walk);
    builtAgain.addEdge(8, 7, EdgeType::Walk);

    REQUIRE_NOTHROW(walker.keepInStep(at(builtAgain, {0.0f, 192.0f})));

    REQUIRE(walker.getCurrentNodeId() == 7);
    REQUIRE(walker.routeFinished());
}

TEST_CASE("A walker whose ground is gone altogether takes no route", "[RouteWalker]")
{
    NavigationGraph navigationGraph = setupPlatform();
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {0.0f, 192.0f}));
    REQUIRE(walker.isAnchored());

    NavigationGraph nothing;

    REQUIRE_NOTHROW(walker.keepInStep(at(nothing, {0.0f, 192.0f})));

    REQUIRE_FALSE(walker.isAnchored());
}

TEST_CASE("A jump leg sends a direction its inputs set, not the way to its target", "[RouteWalker]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, Lower});
    navigationGraph.addNode(1, {96.0f, Upper});
    InputIntentions backingOff;
    backingOff.direction.x = -1.0f;
    backingOff.jumpRequested = backingOff.jumpHeld = true;
    navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, {{0.05f, backingOff}}});
    RouteWalker walker(ArrivalThreshold);
    walker.keepInStep(at(navigationGraph, {0.0f, Lower}));
    walker.takeRouteTo(at(navigationGraph, {0.0f, Lower}), 1);

    ActorFacts inTheAir{
        navigationGraph,
        {0.0f, Lower - 10.0f},
        glm::vec2(8.0f, 13.0f),
        StepHeight,
        std::nullopt,
        ActorContactState{}};

    InputIntentions first = walker.follow(0.01f, at(navigationGraph, {0.0f, Lower}));
    for (int tick = 0; tick < 5; ++tick)
        walker.follow(0.01f, inTheAir);
    InputIntentions after = walker.follow(0.01f, inTheAir);

    REQUIRE(first.direction.x == -1.0f);
    REQUIRE(first.jumpHeld);
    REQUIRE(after.direction.x == 1.0f);
    REQUIRE_FALSE(after.jumpHeld);
}
