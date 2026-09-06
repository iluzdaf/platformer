#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_motion_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/maps.hpp"
#include "helpers/palettes.hpp"
#include "helpers/graph_queries.hpp"
#include "helpers/tiles.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_map.hpp"

TEST_CASE("Walking off a ledge is an edge to the floor below", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) > 0);
}

TEST_CASE("A fall only ever goes down", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Fall)
            REQUIRE(graph.getNode(edge.toId).position.y > graph.getNode(edge.fromId).position.y);
}

TEST_CASE("Falling is not offered where you could walk", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
}

TEST_CASE("A profile that cannot move still falls", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    NavigationProfile profile = profileThatMoves(13.0f, fallerMotionData());

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) > 0);
    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE("A slow actor can still step off a ledge", "[NavigationGraphBuilder][Fall]")
{
    ActorMotionData slow = jumperMotionData();
    slow.moveAbilityData->moveSpeed = 60.0f;

    NavigationProfile profile = profileThatMoves(13.0f, slow);
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) > 0);
}

TEST_CASE("A fall is drawn as the straight drop it is", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Fall)
            REQUIRE(edge.path.empty());
}

TEST_CASE("A node falls to the one below it and nowhere else", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &[id, node] : graph.getNodes())
    {
        int falls = 0;
        for (const auto &edge : graph.getOutgoingEdges(id))
            if (edge.type == EdgeType::Fall)
                ++falls;

        REQUIRE(falls <= 1);
    }
}

TEST_CASE("Nothing falls onto spikes", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveSpikes();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
}

TEST_CASE(
    "Nothing falls through spikes to the floor beneath them",
    "[NavigationGraphBuilder][Fall]")
{
    Placed laid;
    layRow(laid, FloorBelowRow, 0, 19);
    for (int x = 0; x < 20; ++x)
        laid.push_back({glm::ivec2(x, PlatformRow + 1), SpikeTileIndex});
    layRow(laid, PlatformRow, 0, LeftPlatformEnd);
    TileMap tileMap = aTileMap(laid, 20, WideMapHeightTiles, 16, aPaletteWithSpikes());

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
}

TEST_CASE("A ledge gets a node directly below it", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    float floorY = static_cast<float>(FloorBelowRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(nodeJustPastTheLedge(graph, floorY) >= 0);
}

TEST_CASE("The fall from a ledge goes to the node below it", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    float floorY = static_cast<float>(FloorBelowRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());
    int landingId = nodeJustPastTheLedge(graph, floorY);
    REQUIRE(landingId >= 0);

    bool straightDown = false;
    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Fall && edge.toId == landingId)
            straightDown = true;

    REQUIRE(straightDown);
}

TEST_CASE("The node below a ledge is walkable from the floor", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    float floorY = static_cast<float>(FloorBelowRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int landingId = nodeJustPastTheLedge(graph, floorY);
    REQUIRE(landingId >= 0);

    int walkEdges = 0;
    for (const auto &edge : graph.getOutgoingEdges(landingId))
        if (edge.type == EdgeType::Walk)
            ++walkEdges;

    REQUIRE(walkEdges > 0);
}

TEST_CASE("A fall clears the platform it leaves", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Fall)
            continue;

        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        float stepOff = std::abs(to.position.x - from.position.x);
        REQUIRE(stepOff > 0.0f);
        REQUIRE(stepOff < 8.0f);
    }
}

TEST_CASE(
    "A fall onto a floor the graph has no node for is no edge",
    "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    NavigationGraph graph;
    graph.addNode(0, takeOffPosition(tileMap));

    navigation::addFallEdges(graph, tileMap, jumperProfile(), 1);

    REQUIRE(graph.getEdges().empty());
}
