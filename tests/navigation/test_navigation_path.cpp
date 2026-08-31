#include <catch2/catch_test_macros.hpp>
#include <utility>
#include <vector>
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_graph.hpp"

namespace
{
    NavigationGraph setupTwoFloors()
    {
        NavigationGraph navigationGraph;
        navigationGraph.addNode(0, glm::vec2(0.0f, 0.0f));
        navigationGraph.addNode(1, glm::vec2(50.0f, 0.0f));
        navigationGraph.addNode(2, glm::vec2(100.0f, 0.0f));
        navigationGraph.addNode(3, glm::vec2(0.0f, 50.0f));
        navigationGraph.addNode(4, glm::vec2(50.0f, 50.0f));
        navigationGraph.addNode(5, glm::vec2(100.0f, 50.0f));

        for (auto [left, right] :
             {std::pair(0, 1), std::pair(1, 2), std::pair(3, 4), std::pair(4, 5)})
        {
            navigationGraph.addEdge(left, right, EdgeType::Walk);
            navigationGraph.addEdge(right, left, EdgeType::Walk);
        }

        navigationGraph.addEdge(3, 0, EdgeType::Jump);
        navigationGraph.addEdge(2, 5, EdgeType::Fall);

        return navigationGraph;
    }
}

TEST_CASE("A route to where you already are is where you already are", "[NavigationPath]")
{
    REQUIRE(findPath(setupTwoFloors(), 4, 4) == std::vector{4});
}

TEST_CASE("A route along a floor follows it", "[NavigationPath]")
{
    REQUIRE(findPath(setupTwoFloors(), 3, 5) == std::vector{3, 4, 5});
}

TEST_CASE("A route takes the way down that exists", "[NavigationPath]")
{
    REQUIRE(findPath(setupTwoFloors(), 0, 5) == std::vector{0, 1, 2, 5});
}

TEST_CASE("A route does not climb a fall", "[NavigationPath]")
{
    REQUIRE(findPath(setupTwoFloors(), 5, 2) == std::vector{5, 4, 3, 0, 1, 2});
}

TEST_CASE("A route to somewhere unreachable is no route at all", "[NavigationPath]")
{
    NavigationGraph navigationGraph = setupTwoFloors();
    navigationGraph.addNode(6, glm::vec2(500.0f, 0.0f));

    REQUIRE(findPath(navigationGraph, 0, 6).empty());
}

TEST_CASE("A route takes the shorter way, not the one with fewer stops", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, glm::vec2(0.0f, 0.0f));
    navigationGraph.addNode(1, glm::vec2(0.0f, 100.0f));

    navigationGraph.addNode(2, glm::vec2(250.0f, 0.0f));
    navigationGraph.addEdge(0, 2, EdgeType::Walk);
    navigationGraph.addEdge(2, 1, EdgeType::Walk);

    navigationGraph.addNode(3, glm::vec2(0.0f, 25.0f));
    navigationGraph.addNode(4, glm::vec2(0.0f, 50.0f));
    navigationGraph.addNode(5, glm::vec2(0.0f, 75.0f));
    navigationGraph.addEdge(0, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 4, EdgeType::Walk);
    navigationGraph.addEdge(4, 5, EdgeType::Walk);
    navigationGraph.addEdge(5, 1, EdgeType::Walk);

    REQUIRE(findPath(navigationGraph, 0, 1) == std::vector{0, 3, 4, 5, 1});
}

TEST_CASE("A route to a node that is not there is an error", "[NavigationPath]")
{
    REQUIRE_THROWS(findPath(setupTwoFloors(), 0, 99));
}

TEST_CASE("A round trip leaves out what is past a one way drop", "[NavigationPath]")
{
    NavigationGraph navigationGraph = setupTwoFloors();

    REQUIRE(roundTripFrom(navigationGraph, 0) == std::vector{0, 1, 2, 3, 4, 5});

    navigationGraph.addNode(6, glm::vec2(150.0f, 50.0f));
    navigationGraph.addEdge(5, 6, EdgeType::Fall);

    REQUIRE(roundTripFrom(navigationGraph, 0) == std::vector{0, 1, 2, 3, 4, 5});
}

TEST_CASE("A walk stays on the platform", "[NavigationPath]")
{
    REQUIRE(walkableFrom(setupTwoFloors(), 0) == std::vector{0, 1, 2});
    REQUIRE(walkableFrom(setupTwoFloors(), 4) == std::vector{3, 4, 5});
}

TEST_CASE("Two places on one walkable run are on the same run", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addNode(2, {48.0f, 128.0f});
    navigationGraph.addNode(3, {160.0f, 128.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addEdge(2, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 2, EdgeType::Walk);
    navigationGraph.addEdge(1, 3, EdgeType::Fall);

    SECTION("Both ends of the same platform")
    {
        REQUIRE(onTheSameRun(navigationGraph, {16.0f, 96.0f}, {112.0f, 96.0f}));
    }

    SECTION("Out past the node at the end of it, where the ledge still is")
    {
        REQUIRE(onTheSameRun(navigationGraph, {112.0f, 96.0f}, {4.0f, 96.0f}));
    }

    SECTION("The platform below is a run of its own")
    {
        REQUIRE_FALSE(onTheSameRun(navigationGraph, {16.0f, 96.0f}, {48.0f, 128.0f}));
    }

    SECTION("A fall down to it does not join the two")
    {
        REQUIRE_FALSE(onTheSameRun(navigationGraph, {112.0f, 96.0f}, {160.0f, 128.0f}));
    }
}

TEST_CASE("A place is judged by the run under its feet, not the nearest node", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addNode(2, {60.0f, 128.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE_FALSE(onTheSameRun(navigationGraph, {60.0f, 96.0f}, {60.0f, 128.0f}));
}

TEST_CASE("A beat between two ends of one platform can be walked", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 192.0f});
    navigationGraph.addNode(1, {112.0f, 192.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(canPatrolBetween(navigationGraph, {16.0f, 192.0f}, {112.0f, 192.0f}));
}

TEST_CASE("A beat ending somewhere with no way back cannot", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 128.0f});
    navigationGraph.addNode(1, {112.0f, 128.0f});
    navigationGraph.addNode(2, {112.0f, 192.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addEdge(1, 2, EdgeType::Fall);

    REQUIRE(canPatrolBetween(navigationGraph, {16.0f, 128.0f}, {112.0f, 128.0f}));
    REQUIRE_FALSE(canPatrolBetween(navigationGraph, {16.0f, 128.0f}, {112.0f, 192.0f}));
}

TEST_CASE("A beat naming one place twice is walkable by standing still", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 192.0f});

    REQUIRE(canPatrolBetween(navigationGraph, {16.0f, 192.0f}, {16.0f, 192.0f}));
}

TEST_CASE("A beat against a graph with no nodes cannot be walked", "[NavigationPath]")
{
    NavigationGraph navigationGraph;

    REQUIRE_FALSE(canPatrolBetween(navigationGraph, {16.0f, 192.0f}, {112.0f, 192.0f}));
}

TEST_CASE("A place on a run keeps the spot asked for", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(placeOnTheRun(navigationGraph, {88.0f, 96.0f}) == glm::vec2(88.0f, 96.0f));
}

TEST_CASE("A place past the end of a run is the end of it", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(placeOnTheRun(navigationGraph, {400.0f, 96.0f}) == glm::vec2(112.0f, 96.0f));
    REQUIRE(placeOnTheRun(navigationGraph, {-40.0f, 96.0f}) == glm::vec2(16.0f, 96.0f));
}

TEST_CASE("A place stays on the run it was asked about", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addNode(2, {64.0f, 128.0f});
    navigationGraph.addNode(3, {160.0f, 128.0f});
    navigationGraph.addEdge(2, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 2, EdgeType::Walk);

    REQUIRE(placeOnTheRun(navigationGraph, {72.0f, 96.0f}).y == 96.0f);
    REQUIRE(placeOnTheRun(navigationGraph, {72.0f, 128.0f}).y == 128.0f);
}

TEST_CASE("A place asked for off any run falls to the run nearest it", "[NavigationPath]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(placeOnTheRun(navigationGraph, {64.0f, 32.0f}) == glm::vec2(64.0f, 96.0f));
    REQUIRE(placeOnTheRun(navigationGraph, {400.0f, 32.0f}) == glm::vec2(112.0f, 96.0f));
}
