#include <optional>
#include <catch2/catch_test_macros.hpp>
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_place.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_graph.hpp"

TEST_CASE("Two places on one walkable run are on the same run", "[NavigationPlace]")
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

TEST_CASE("A place is judged by the run under its feet, not the nearest node", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addNode(2, {60.0f, 128.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE_FALSE(onTheSameRun(navigationGraph, {60.0f, 96.0f}, {60.0f, 128.0f}));
}

TEST_CASE("A beat between two ends of one platform can be walked", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 192.0f});
    navigationGraph.addNode(1, {112.0f, 192.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(canPatrolBetween(navigationGraph, {16.0f, 192.0f}, {112.0f, 192.0f}));
}

TEST_CASE("A beat ending somewhere with no way back cannot", "[NavigationPlace]")
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

TEST_CASE("A beat naming one place twice is walkable by standing still", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 192.0f});

    REQUIRE(canPatrolBetween(navigationGraph, {16.0f, 192.0f}, {16.0f, 192.0f}));
}

TEST_CASE("A beat against a graph with no nodes cannot be walked", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;

    REQUIRE_FALSE(canPatrolBetween(navigationGraph, {16.0f, 192.0f}, {112.0f, 192.0f}));
}

TEST_CASE("A place on a path keeps the spot asked for", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(placeOnThePath(navigationGraph, {88.0f, 96.0f})->position == glm::vec2(88.0f, 96.0f));
}

TEST_CASE("A place past the end of a path is the end of it", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(placeOnThePath(navigationGraph, {400.0f, 96.0f})->position == glm::vec2(112.0f, 96.0f));
    REQUIRE(placeOnThePath(navigationGraph, {-40.0f, 96.0f})->position == glm::vec2(16.0f, 96.0f));
}

TEST_CASE("A place stays on the path it was asked about", "[NavigationPlace]")
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

    REQUIRE(placeOnThePath(navigationGraph, {72.0f, 96.0f})->position.y == 96.0f);
    REQUIRE(placeOnThePath(navigationGraph, {72.0f, 128.0f})->position.y == 128.0f);
}

TEST_CASE("A place asked for off every path falls to the path nearest it", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    REQUIRE(placeOnThePath(navigationGraph, {64.0f, 32.0f})->position == glm::vec2(64.0f, 96.0f));
    REQUIRE(placeOnThePath(navigationGraph, {400.0f, 32.0f})->position == glm::vec2(112.0f, 96.0f));
}

TEST_CASE("A place can be partway up a climb", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {16.0f, 16.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Climb);
    navigationGraph.addEdge(1, 0, EdgeType::Climb);

    std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, {24.0f, 64.0f});

    REQUIRE(place);
    REQUIRE(place->position == glm::vec2(16.0f, 64.0f));
}

TEST_CASE("A place beside a wall prefers the wall to the floor below it", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);
    navigationGraph.addNode(2, {16.0f, 16.0f});
    navigationGraph.addEdge(0, 2, EdgeType::Climb);
    navigationGraph.addEdge(2, 0, EdgeType::Climb);

    std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, {24.0f, 64.0f});

    REQUIRE(place);
    REQUIRE(place->position == glm::vec2(16.0f, 64.0f));
}

TEST_CASE("A jump is not somewhere to stop partway", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 192.0f});
    navigationGraph.addNode(1, {96.0f, 96.0f});
    navigationGraph.addEdge({0, 1, EdgeType::Jump, {}, 0.2f});
    navigationGraph.addEdge({1, 0, EdgeType::Jump, {}, 0.2f});

    std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, {48.0f, 144.0f});

    REQUIRE(place);
    REQUIRE(
        (place->position == glm::vec2(0.0f, 192.0f) || place->position == glm::vec2(96.0f, 96.0f)));
}

TEST_CASE("A beat is judged by the path it lands on, not the node it is near", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {0.0f, 192.0f});
    navigationGraph.addNode(1, {96.0f, 192.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    navigationGraph.addNode(2, {0.0f, 100.0f});
    navigationGraph.addNode(3, {96.0f, 100.0f});
    navigationGraph.addEdge(2, 3, EdgeType::Walk);
    navigationGraph.addEdge(3, 2, EdgeType::Walk);

    navigationGraph.addNode(4, {48.0f, 110.0f});
    navigationGraph.addEdge({0, 4, EdgeType::Jump, {}, 0.2f});
    navigationGraph.addEdge({4, 0, EdgeType::Jump, {}, 0.2f});

    REQUIRE(nearestNodeTo(navigationGraph, {48.0f, 100.0f}) == 4);
    REQUIRE(placeOnThePath(navigationGraph, {48.0f, 100.0f})->position == glm::vec2(48.0f, 100.0f));

    REQUIRE_FALSE(canPatrolBetween(navigationGraph, {10.0f, 192.0f}, {48.0f, 100.0f}));
    REQUIRE(canPatrolBetween(navigationGraph, {10.0f, 192.0f}, {48.0f, 110.0f}));
}

TEST_CASE("The end of a path towards somewhere is the end nearer it", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, {64.0f, 96.0f});

    REQUIRE(place);
    REQUIRE(endOfThePathTowards(navigationGraph, *place, {300.0f, 96.0f}) == 1);
    REQUIRE(endOfThePathTowards(navigationGraph, *place, {-300.0f, 96.0f}) == 0);
}

TEST_CASE("The end of a path that is one node is that node", "[NavigationPlace]")
{
    NavigationGraph navigationGraph;
    navigationGraph.addNode(0, {16.0f, 96.0f});
    navigationGraph.addNode(1, {112.0f, 96.0f});
    navigationGraph.addEdge(0, 1, EdgeType::Walk);
    navigationGraph.addEdge(1, 0, EdgeType::Walk);

    std::optional<PlaceOnThePath> place = placeOnThePath(navigationGraph, {16.0f, 96.0f});

    REQUIRE(place);
    REQUIRE(place->fromId == place->toId);
    REQUIRE(endOfThePathTowards(navigationGraph, *place, {300.0f, 96.0f}) == 0);
}
