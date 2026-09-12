#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/actors.hpp"
#include "helpers/tiles.hpp"
#include "helpers/navigation_maps.hpp"
#include "helpers/graph_queries.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_map.hpp"

TEST_CASE("A profile that cannot jump gets no jump edges", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE("A jumper crosses a gap it can clear", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = twoPlatformsApart(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(jumpsAcrossTo(graph, takeOffPosition(tileMap), landingPosition(tileMap, GapTiles)));
}

TEST_CASE("A jumper crosses a gap in both directions", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = twoPlatformsApart(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(jumpsAcrossTo(graph, landingPosition(tileMap, GapTiles), takeOffPosition(tileMap)));
}

TEST_CASE("A jumper does not cross a gap beyond its reach", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(14, 0, 30);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE("A jumper reaches a ledge two tiles up", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 2;
    constexpr int RowsUp = 2;
    TileMap tileMap = twoPlatformsApart(GapTiles, RowsUp);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(
        jumpsAcrossTo(graph, takeOffPosition(tileMap), landingPosition(tileMap, GapTiles, RowsUp)));
}

TEST_CASE("A jumper does not reach a ledge six tiles up", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 2;
    constexpr int RowsUp = 6;
    TileMap tileMap = twoPlatformsApart(GapTiles, RowsUp);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE_FALSE(hasEdgeBetween(
        graph,
        takeOffPosition(tileMap),
        landingPosition(tileMap, GapTiles, RowsUp),
        EdgeType::Jump));
}

TEST_CASE(
    "A ceiling over the gap blocks a jump that would otherwise clear it",
    "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    Placed laid = twoPlatforms(GapTiles);
    layRow(laid, PlatformRow - 2, LeftPlatformEnd + 1, LeftPlatformEnd + GapTiles);
    TileMap tileMap = aTileMap(laid, 20, WideMapHeightTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE_FALSE(hasEdgeBetween(
        graph, takeOffPosition(tileMap), landingPosition(tileMap, GapTiles), EdgeType::Jump));
}

TEST_CASE("A jumper still walks the platform it stands on", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Walk) > 0);
}

TEST_CASE("A jump edge carries the arc that produced it", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = twoPlatformsApart(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        REQUIRE(edge.path.size() > 2);
        REQUIRE(edge.path.front() == graph.getNode(edge.fromId).feet);
        REQUIRE(edge.path.back().y == graph.getNode(edge.toId).feet.y);
    }
}

TEST_CASE("An arc on an edge rises above both of its ends", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = twoPlatformsApart(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        float highest = edge.path.front().y;
        for (const glm::vec2 &position : edge.path)
            highest = std::min(highest, position.y);

        REQUIRE(highest < graph.getNode(edge.fromId).feet.y);
        REQUIRE(highest < graph.getNode(edge.toId).feet.y);
    }
}

TEST_CASE("A walk edge is drawn straight", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Walk)
            REQUIRE(edge.path.empty());
}

TEST_CASE(
    "A way up does not depend on something having fallen there",
    "[NavigationGraphBuilder][Jump]")
{
    Placed laid;
    layRow(laid, 12, 1, 8);
    layRow(laid, 12, 12, 18);
    layRow(laid, 6, 1, 4);
    TileMap tileMap = aTileMap(laid, 20, 16);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int landings = 0;
    for (const auto &[id, node] : graph.getNodes())
        if (node.kind == NodeKind::Landing)
            ++landings;

    INFO("nothing fell anywhere, so this proves nothing");
    REQUIRE(landings > 0);

    int jumps = 0;
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        ++jumps;
        NavigationNode from = graph.getNode(edge.fromId);
        INFO("jump from node " << edge.fromId << " at " << from.feet.x << "," << from.feet.y);
        REQUIRE(from.kind == NodeKind::OnFoot);
    }

    REQUIRE(jumps > 0);
}

TEST_CASE("A jump edge records the hold that made it", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Jump)
            REQUIRE(edge.holdDuration > 0.0f);
}

TEST_CASE("A walk edge is held for nothing", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Walk)
            REQUIRE(edge.holdDuration == 0.0f);
}

TEST_CASE("A gap needing less than a full jump records less", "[NavigationGraphBuilder][Jump]")
{
    NavigationProfile profile = jumperProfile();
    TileMap tileMap = twoPlatformsApart(1);

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Jump)
            REQUIRE(edge.holdDuration <= profile.jumpArcs.front().holdDuration);
}

TEST_CASE("A jump is the smallest one that reaches", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = ledgesAboveAFloor();
    float ledgeY = static_cast<float>(PlatformRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int acrossTheGap =
        graph.nodeAtPosition(glm::vec2(static_cast<float>(NearLedgeEnd + 1) * 16.0f, ledgeY))
            .value_or(-1);
    int farSide = graph.nodeAtPosition(glm::vec2(static_cast<float>(FarLedgeStart) * 16.0f, ledgeY))
                      .value_or(-1);
    int longWayOff = graph.nodeAtPosition(glm::vec2(0.0f, ledgeY)).value_or(-1);
    REQUIRE(acrossTheGap >= 0);
    REQUIRE(farSide >= 0);
    REQUIRE(longWayOff >= 0);

    const NavigationEdge *there = onlyJumpFrom(graph, acrossTheGap);
    const NavigationEdge *back = onlyJumpFrom(graph, farSide);
    REQUIRE(there);
    REQUIRE(back);
    REQUIRE(there->holdDuration == back->holdDuration);

    const NavigationEdge *further = onlyJumpFrom(graph, longWayOff);
    REQUIRE(further);
    REQUIRE(there->holdDuration < further->holdDuration);
}

TEST_CASE("A jump crosses to a platform once", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = ledgesAboveAFloor();
    float ledgeY = static_cast<float>(PlatformRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int takeOffId = graph.nodeAtPosition(glm::vec2(0.0f, ledgeY)).value_or(-1);
    int nearEdgeId =
        graph.nodeAtPosition(glm::vec2(static_cast<float>(FarLedgeStart) * 16.0f, ledgeY))
            .value_or(-1);
    REQUIRE(takeOffId >= 0);
    REQUIRE(nearEdgeId >= 0);
    REQUIRE(
        graph.hasNodeAtPosition(glm::vec2(static_cast<float>(FarLedgeEnd + 1) * 16.0f, ledgeY)));

    const NavigationEdge *only = onlyJumpFrom(graph, takeOffId);
    REQUIRE(only);
    REQUIRE(graph.getNode(only->toId).feet.y == ledgeY);
    REQUIRE(graph.getNode(only->toId).feet.x >= static_cast<float>(FarLedgeStart) * 16.0f - 0.5f);
}

TEST_CASE("Nothing jumps to where it could walk", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = ledgesAboveAFloor();
    float floorY = static_cast<float>(DeepFloorRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    size_t onTheFloor = 0;
    for (const auto &[id, node] : graph.getNodes())
        if (std::abs(node.feet.y - floorY) < 0.5f)
            ++onTheFloor;

    REQUIRE(onTheFloor >= 4);

    int jumpsAlongTheFloor = 0;
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        if (std::abs(graph.getNode(edge.fromId).feet.y - floorY) < 0.5f &&
            std::abs(graph.getNode(edge.toId).feet.y - floorY) < 0.5f)
            ++jumpsAlongTheFloor;
    }

    REQUIRE(jumpsAlongTheFloor == 0);
}

TEST_CASE("A jump still falling after ten seconds is no jump", "[NavigationGraphBuilder][Jump]")
{
    constexpr int TallerThanTenSecondsOfFalling = 420;
    Placed laid;
    layRow(laid, 1, 0, LeftPlatformEnd);
    TileMap tileMap = aTileMap(laid, 20, TallerThanTenSecondsOfFalling);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE(
    "The easiest jump is kept whichever order the arcs are tried in",
    "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = twoPlatformsApart(2);
    NavigationProfile longestHoldFirst = jumperProfile();
    NavigationProfile shortestHoldFirst = longestHoldFirst;
    std::ranges::reverse(shortestHoldFirst.jumpArcs);
    REQUIRE(longestHoldFirst.jumpArcs.size() > 1);

    auto holdsOfJumps = [](const NavigationGraph &graph)
    {
        std::vector<float> holds;
        for (const auto &edge : graph.getEdges())
            if (edge.type == EdgeType::Jump)
                holds.push_back(edge.holdDuration);
        std::ranges::sort(holds);
        return holds;
    };

    REQUIRE(
        holdsOfJumps(buildNavigationGraph(tileMap, longestHoldFirst)) ==
        holdsOfJumps(buildNavigationGraph(tileMap, shortestHoldFirst)));
}

TEST_CASE(
    "A jump onto a floor the graph has no node for is no edge",
    "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    NavigationGraph graph;
    graph.addNode(0, takeOffPosition(tileMap));
    glm::vec2 comesDown(takeOffPosition(tileMap).x + 40.0f, static_cast<float>(FloorBelowRow * 16));
    std::vector<navigation::ChosenJump> jumps{{0, {takeOffPosition(tileMap), comesDown}, 0.2f}};

    navigation::addJumpEdges(graph, tileMap, 1, jumps);

    REQUIRE(graph.getEdges().empty());
}
