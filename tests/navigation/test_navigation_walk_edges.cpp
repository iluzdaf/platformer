#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/game_data.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/actors.hpp"
#include "helpers/maps.hpp"
#include "helpers/graph_queries.hpp"
#include "helpers/tiles.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "tile_map/tile_map.hpp"

TEST_CASE("A floor gives a profile somewhere to walk", "[NavigationGraphBuilder]")
{
    TileMap tileMap = aFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE_FALSE(graph.getEdges().empty());
    for (const auto &edge : graph.getEdges())
        REQUIRE(edge.type == EdgeType::Walk);
}

TEST_CASE("A profile too tall for the headroom has nowhere to stand", "[NavigationGraphBuilder]")
{
    TileMap tileMap = aFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(20.0f));

    REQUIRE(nodesOnTheFloor(graph, tileMap) == 0);
}

TEST_CASE("A profile that fits the headroom still walks under it", "[NavigationGraphBuilder]")
{
    TileMap tileMap = aFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(nodesOnTheFloor(graph, tileMap) == 2);
}

TEST_CASE("A profile that fits walks the length of a one tile corridor", "[NavigationGraphBuilder]")
{
    TileMap tileMap = aFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(walksTheFloorEndToEnd(graph, tileMap));
}

TEST_CASE("Headroom rounds up to whole tiles", "[NavigationGraphBuilder]")
{
    TileMap tileMap = aFloorUnderOneTileOfHeadroom();
    float tileSize = static_cast<float>(tileMap.getTileSize());

    SECTION("a collider exactly one tile tall needs one tile")
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(tileSize));
        REQUIRE(nodesOnTheFloor(graph, tileMap) == 2);
    }

    SECTION("a collider one pixel taller needs two")
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(tileSize + 1.0f));
        REQUIRE(nodesOnTheFloor(graph, tileMap) == 0);
    }
}

TEST_CASE("A corridor that pinches stops a profile that no longer fits", "[NavigationGraphBuilder]")
{
    TileMap tileMap = aCorridorThatPinches();

    SECTION("a profile needing two tiles cannot pass the pinch")
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(20.0f));

        REQUIRE(nodesOnTheFloor(graph, tileMap) == 4);
        REQUIRE_FALSE(anEdgeSpansThePinch(graph, tileMap));
    }

    SECTION("a profile needing one tile walks straight through")
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

        REQUIRE(nodesOnTheFloor(graph, tileMap) == 2);
        REQUIRE(anEdgeSpansThePinch(graph, tileMap));
    }
}

TEST_CASE("Walk edges are bidirectional along a floor", "[NavigationGraphBuilder]")
{
    Placed laid;
    layRow(laid, 5, 2, 4);
    TileMap tileMap = aTileMap(laid);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE(nodeXsOnRow(graph, 80.0f) == std::vector<float>{32.0f, 80.0f});
    REQUIRE(hasEdgeBetween(graph, 32.0f, 80.0f, 80.0f));
    REQUIRE(hasEdgeBetween(graph, 80.0f, 32.0f, 80.0f));

    for (const auto &edge : graph.getEdges())
        REQUIRE(edge.type == EdgeType::Walk);
}

TEST_CASE("No walk edge spans a gap between floors", "[NavigationGraphBuilder]")
{
    Placed laid;
    layRow(laid, 5, 0, 2);
    layRow(laid, 5, 6, 9);
    TileMap tileMap = aTileMap(laid);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE_FALSE(hasEdgeBetween(graph, 48.0f, 96.0f, 80.0f));
    REQUIRE_FALSE(hasEdgeBetween(graph, 96.0f, 48.0f, 80.0f));
    REQUIRE_FALSE(isReachable(graph, {48.0f, 80.0f}, {96.0f, 80.0f}));

    REQUIRE(isReachable(graph, {0.0f, 80.0f}, {48.0f, 80.0f}));
    REQUIRE(isReachable(graph, {96.0f, 80.0f}, {160.0f, 80.0f}));
}

TEST_CASE("A floor is one run, and a gap makes it two", "[NavigationGraphBuilder]")
{
    Placed laid;
    layRow(laid, 5, 0, 2);
    layRow(laid, 5, 6, 9);
    TileMap tileMap = aTileMap(laid);
    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    std::vector<std::vector<int>> runs = navigation::walkRuns(graph, tileMap, 1);

    REQUIRE(runs.size() == 2);
    for (const std::vector<int> &run : runs)
        for (size_t at = 1; at < run.size(); ++at)
            REQUIRE(graph.getNode(run[at - 1]).feet.x < graph.getNode(run[at]).feet.x);
}

TEST_CASE("No walk edge passes through a blocked tile", "[NavigationGraphBuilder]")
{
    Placed laid;
    layRow(laid, 5, 0, 5);
    laid.push_back({glm::ivec2(3, 4), 1});
    TileMap tileMap = aTileMap(laid);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    for (const auto &edge : graph.getEdges())
    {
        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        float low = std::min(from.feet.x, to.feet.x);
        float high = std::max(from.feet.x, to.feet.x);
        REQUIRE_FALSE((low <= 48.0f && high >= 64.0f));
    }

    REQUIRE_FALSE(isReachable(graph, {0.0f, 80.0f}, {96.0f, 80.0f}));
}

TEST_CASE("Floors on different rows are not connected", "[NavigationGraphBuilder]")
{
    Placed laid;
    layRow(laid, 5, 0, 3);
    layRow(laid, 8, 0, 3);
    TileMap tileMap = aTileMap(laid);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    for (const auto &edge : graph.getEdges())
        REQUIRE(graph.getNode(edge.fromId).feet.y == graph.getNode(edge.toId).feet.y);
}

TEST_CASE(
    "Every shipped level gives the standard actor somewhere to walk",
    "[NavigationGraphBuilder][Level]")
{
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        TileMap tileMap = tilesOfLevel(entry.path().string());
        INFO("level " << entry.path().filename().string() << " has no walkable graph");
        REQUIRE_FALSE(buildNavigationGraph(tileMap, standardProfile()).getEdges().empty());
    }
}

TEST_CASE("Where a node sits on a platform", "[NavigationGraphBuilder]")
{
    float tileSize = static_cast<float>(aTileMap().getTileSize());

    SECTION("Single Tile Platform at left side of TileMap")
    {
        TileMap tileMap = aTileMap({{{0, 9}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform at right side of TileMap")
    {
        TileMap tileMap = aTileMap({{{9, 9}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({9 * tileSize + tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform where both sides are cliffs")
    {
        TileMap tileMap = aTileMap({{{1, 9}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform where left side is a cliff and right side is a wall")
    {
        TileMap tileMap = aTileMap({{{2, 0}, 1}, {{2, 1}, 1}, {{1, 1}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("Single Tile Platform where right side is a cliff and left side is a wall")
    {
        TileMap tileMap = aTileMap({{{0, 0}, 1}, {{0, 1}, 1}, {{1, 1}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("Single Tile Platform where both sides are walls")
    {
        TileMap tileMap =
            aTileMap({{{0, 0}, 1}, {{0, 1}, 1}, {{1, 1}, 1}, {{2, 0}, 1}, {{2, 1}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("2 Tile Platforms")
    {
        Placed laid;
        layRow(laid, 1, 0, 1);
        TileMap tileMap = aTileMap(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({2 * tileSize, 1 * tileSize}));
    }

    SECTION("3 Tile Platforms")
    {
        Placed laid;
        layRow(laid, 1, 0, 2);
        TileMap tileMap = aTileMap(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({3 * tileSize, 1 * tileSize}));
    }

    SECTION("5 Tile Platforms")
    {
        Placed laid;
        layRow(laid, 1, 0, 4);
        TileMap tileMap = aTileMap(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({5 * tileSize, 1 * tileSize}));
    }

    SECTION("10 Tile Platforms")
    {
        Placed laid;
        layRow(laid, 1, 0, 9);
        TileMap tileMap = aTileMap(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({10 * tileSize, 1 * tileSize}));
    }

    SECTION("No nodes")
    {
        NavigationGraph navigationGraph = buildNavigationGraph(aTileMap(), standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 0);
    }

    SECTION("No walkable tiles")
    {
        Placed laid;
        layRow(laid, 0, 0, 2);
        TileMap tileMap = aTileMap(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 0);
    }
}

TEST_CASE("Every node stands on the top of a tile", "[NavigationGraphBuilder][Level]")
{
    GameData gameData = loadGameData();
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph = buildNavigationGraph(
        tileMap, buildNavigationProfile(gameData.npcData.at("spider").actorData));

    float tileSize = static_cast<float>(tileMap.getTileSize());
    for (const auto &[id, node] : graph.getNodes())
    {
        if (node.kind == NodeKind::OnWall)
            continue;

        INFO("node " << id << " at " << node.feet.x << "," << node.feet.y);
        REQUIRE(std::fmod(node.feet.y, tileSize) == 0.0f);

        glm::ivec2 under = tileMap.tileContaining(node.feet + glm::vec2(0.0f, 1.0f));
        glm::ivec2 justBehind = tileMap.tileContaining(node.feet + glm::vec2(-1.0f, 1.0f));
        REQUIRE(
            (tileMap.getTileAtTilePosition(under).isSolid() ||
             tileMap.getTileAtTilePosition(justBehind).isSolid()));
    }
}

TEST_CASE("A place is not walkable to itself, nor to another row", "[NavigationGraphBuilder]")
{
    Placed laid;
    layRow(laid, 5, 0, 9);
    layRow(laid, 3, 0, 9);
    TileMap tileMap = aTileMap(laid);
    glm::vec2 onTheLowerFloor = tileMap.feetOnTile(glm::ivec2(2, 4));
    glm::vec2 onTheUpperFloor = tileMap.feetOnTile(glm::ivec2(5, 2));

    REQUIRE_FALSE(navigation::isWalkableBetween(tileMap, onTheLowerFloor, onTheLowerFloor, 1));
    REQUIRE_FALSE(navigation::isWalkableBetween(tileMap, onTheLowerFloor, onTheUpperFloor, 1));
}
