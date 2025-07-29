#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <unordered_set>
#include "navigation/navigation_graph.hpp"

TEST_CASE("Add Nodes and Edges", "[NavigationGraph]")
{
    NavigationGraph graph;
    graph.addNode(0, {0, 0});
    graph.addNode(1, {10, 0});
    graph.addNode(2, {20, 0});
    graph.addEdge(0, 1, EdgeType::Walk);
    graph.addEdge(1, 2, EdgeType::Walk);

    REQUIRE(graph.getNodes().size() == 3);
    REQUIRE(graph.getEdges().size() == 2);
}

TEST_CASE("Get Node by ID", "[NavigationGraph]")
{
    NavigationGraph graph;
    graph.addNode(0, {0, 0});
    graph.addNode(1, {10, 0});

    SECTION("Valid node ID returns correct node")
    {
        auto node = graph.getNode(1);
        REQUIRE(node.id == 1);
        REQUIRE(node.position == glm::vec2(10, 0));
    }

    SECTION("Invalid node ID throws exception")
    {
        REQUIRE_THROWS_WITH(graph.getNode(-1), "Invalid node ID");
        REQUIRE_THROWS_WITH(graph.getNode(100), "Invalid node ID");
    }
}

TEST_CASE("Get Outgoing Edges", "[NavigationGraph]")
{
    NavigationGraph graph;
    graph.addNode(0, {0, 0});
    graph.addNode(1, {10, 0});
    graph.addNode(2, {20, 0});
    graph.addEdge(0, 1, EdgeType::Walk);
    graph.addEdge(0, 2, EdgeType::Jump);
    graph.addEdge(1, 2, EdgeType::Walk);

    SECTION("Returns correct outgoing edges from node A")
    {
        const auto &edges = graph.getOutgoingEdges(0);
        REQUIRE(edges.size() == 2);

        std::unordered_set<int> destinations;
        for (const auto &edge : edges)
        {
            destinations.insert(edge.toId);
        }

        REQUIRE(destinations.contains(1));
        REQUIRE(destinations.contains(2));
    }

    SECTION("Returns empty vector for node with no outgoing edges")
    {
        const auto &edges = graph.getOutgoingEdges(2);
        REQUIRE(edges.empty());
    }

    SECTION("Returns empty vector for unknown node")
    {
        const auto &edges = graph.getOutgoingEdges(99);
        REQUIRE(edges.empty());
    }
}

TEST_CASE("Cannot add self-loop edge", "[NavigationGraph]")
{
    NavigationGraph graph;
    graph.addNode(0, {0, 0});
    REQUIRE_THROWS_WITH(graph.addEdge(0, 0, EdgeType::Walk), "Cannot add self-loop edge: fromId == toId");
}

TEST_CASE("addEdge throws on invalid node IDs", "[NavigationGraph]")
{
    NavigationGraph graph;
    graph.addNode(0, {0, 0});
    graph.addNode(1, {10, 0});

    SECTION("Throws if fromNodeId is invalid")
    {
        REQUIRE_THROWS_AS(graph.addEdge(-1, 1, EdgeType::Walk), std::runtime_error);
        REQUIRE_THROWS_AS(graph.addEdge(100, 1, EdgeType::Walk), std::runtime_error);
    }

    SECTION("Throws if toNodeId is invalid")
    {
        REQUIRE_THROWS_AS(graph.addEdge(0, -1, EdgeType::Walk), std::runtime_error);
        REQUIRE_THROWS_AS(graph.addEdge(0, 100, EdgeType::Walk), std::runtime_error);
    }

    SECTION("Throws if both IDs are invalid")
    {
        REQUIRE_THROWS_AS(graph.addEdge(-5, 500, EdgeType::Jump), std::runtime_error);
    }
}