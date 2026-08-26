#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <filesystem>
#include <set>
#include <vector>
#include "navigation/navigation_graph_builder.hpp"
#include "game/tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"

namespace
{
    constexpr int FloorRow = 6;
    constexpr int CeilingRow = 4;
    constexpr int MapWidthTiles = 10;

    NavigationProfile profileOfHeight(float height)
    {
        NavigationProfile profile;
        profile.colliderSize = glm::vec2(8.0f, height);
        return profile;
    }

    NavigationProfile standardProfile()
    {
        return profileOfHeight(13.0f);
    }

    TileMap setupFloor()
    {
        TileMap tileMap = setupTileMap();
        for (int x = 0; x < MapWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, FloorRow), 1);
        return tileMap;
    }

    TileMap setupFloorUnderOneTileOfHeadroom()
    {
        TileMap tileMap = setupFloor();
        for (int x = 0; x < MapWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, CeilingRow), 1);
        return tileMap;
    }

    constexpr int HighCeilingRow = 2;
    constexpr int PinchColumn = 5;

    TileMap setupCorridorThatPinches()
    {
        TileMap tileMap = setupFloor();
        for (int x = 0; x < MapWidthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, HighCeilingRow), 1);

        tileMap.setTileIndex(glm::ivec2(PinchColumn, FloorRow - 2), 1);
        return tileMap;
    }

    bool anEdgeSpansThePinch(const NavigationGraph &graph, const TileMap &tileMap)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        float pinchX = (static_cast<float>(PinchColumn) + 0.5f) * tileSize;
        float floorY = static_cast<float>(FloorRow) * tileSize;
        for (const auto &edge : graph.getEdges())
        {
            NavigationNode from = graph.getNode(edge.fromId);
            NavigationNode to = graph.getNode(edge.toId);
            if (from.position.y != floorY || to.position.y != floorY)
                continue;

            float low = std::min(from.position.x, to.position.x);
            float high = std::max(from.position.x, to.position.x);
            if (low < pinchX && high > pinchX)
                return true;
        }
        return false;
    }

    void layFloor(TileMap &tileMap, int groundY, int fromX, int toX)
    {
        for (int x = fromX; x <= toX; ++x)
            tileMap.setTileIndex(glm::ivec2(x, groundY), 1);
    }

    std::vector<float> nodeXsOnRow(const NavigationGraph &graph, float y)
    {
        std::vector<float> xs;
        for (const auto &[id, node] : graph.getNodes())
            if (node.position.y == y)
                xs.push_back(node.position.x);
        std::sort(xs.begin(), xs.end());
        return xs;
    }

    int nodeIdAt(const NavigationGraph &graph, float x, float y)
    {
        for (const auto &[id, node] : graph.getNodes())
            if (node.position == glm::vec2(x, y))
                return id;
        return -1;
    }

    bool hasEdgeBetween(const NavigationGraph &graph, float fromX, float toX, float y)
    {
        for (const auto &edge : graph.getEdges())
        {
            NavigationNode from = graph.getNode(edge.fromId);
            NavigationNode to = graph.getNode(edge.toId);
            if (from.position == glm::vec2(fromX, y) && to.position == glm::vec2(toX, y))
                return true;
        }
        return false;
    }

    bool isReachable(const NavigationGraph &graph, glm::vec2 from, glm::vec2 to)
    {
        int fromId = nodeIdAt(graph, from.x, from.y);
        int toId = nodeIdAt(graph, to.x, to.y);
        if (fromId < 0 || toId < 0)
            return false;

        std::set<int> seen{fromId};
        std::vector<int> pending{fromId};
        while (!pending.empty())
        {
            int nodeId = pending.back();
            pending.pop_back();
            if (nodeId == toId)
                return true;

            for (const auto &edge : graph.getOutgoingEdges(nodeId))
                if (seen.insert(edge.toId).second)
                    pending.push_back(edge.toId);
        }
        return false;
    }

    size_t nodesOnTheFloor(const NavigationGraph &graph, const TileMap &tileMap)
    {
        float floorY = static_cast<float>(FloorRow * tileMap.getTileSize());
        size_t count = 0;
        for (const auto &[id, node] : graph.getNodes())
            if (node.position.y == floorY)
                ++count;
        return count;
    }

    bool walksTheFloorEndToEnd(const NavigationGraph &graph, const TileMap &tileMap)
    {
        float floorY = static_cast<float>(FloorRow * tileMap.getTileSize());
        for (const auto &edge : graph.getEdges())
        {
            if (edge.type != EdgeType::Walk)
                continue;

            NavigationNode from = graph.getNode(edge.fromId);
            NavigationNode to = graph.getNode(edge.toId);
            if (from.position.y == floorY && to.position.y == floorY)
                return true;
        }
        return false;
    }
}

TEST_CASE("A floor gives a profile somewhere to walk", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE_FALSE(graph.getEdges().empty());
    for (const auto &edge : graph.getEdges())
        REQUIRE(edge.type == EdgeType::Walk);
}

TEST_CASE("A profile too tall for the headroom has nowhere to stand", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, profileOfHeight(20.0f));

    REQUIRE(nodesOnTheFloor(graph, tileMap) == 0);
}

TEST_CASE("A profile that fits the headroom still walks under it", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(nodesOnTheFloor(graph, tileMap) == 2);
}

TEST_CASE("A profile that fits walks the length of a one tile corridor", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(walksTheFloorEndToEnd(graph, tileMap));
}

TEST_CASE("Headroom rounds up to whole tiles", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupFloorUnderOneTileOfHeadroom();
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
    TileMap tileMap = setupCorridorThatPinches();

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
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 2, 4);

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
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 2);
    layFloor(tileMap, 5, 6, 9);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE_FALSE(hasEdgeBetween(graph, 48.0f, 96.0f, 80.0f));
    REQUIRE_FALSE(hasEdgeBetween(graph, 96.0f, 48.0f, 80.0f));
    REQUIRE_FALSE(isReachable(graph, {48.0f, 80.0f}, {96.0f, 80.0f}));

    REQUIRE(isReachable(graph, {0.0f, 80.0f}, {48.0f, 80.0f}));
    REQUIRE(isReachable(graph, {96.0f, 80.0f}, {160.0f, 80.0f}));
}

TEST_CASE("No walk edge passes through a blocked tile", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 5);
    tileMap.setTileIndex(glm::ivec2(3, 4), 1);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    for (const auto &edge : graph.getEdges())
    {
        NavigationNode from = graph.getNode(edge.fromId);
        NavigationNode to = graph.getNode(edge.toId);
        float low = std::min(from.position.x, to.position.x);
        float high = std::max(from.position.x, to.position.x);
        REQUIRE_FALSE((low <= 48.0f && high >= 64.0f));
    }

    REQUIRE_FALSE(isReachable(graph, {0.0f, 80.0f}, {96.0f, 80.0f}));
}

TEST_CASE("Floors on different rows are not connected", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupTileMap();
    layFloor(tileMap, 5, 0, 3);
    layFloor(tileMap, 8, 0, 3);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    for (const auto &edge : graph.getEdges())
        REQUIRE(graph.getNode(edge.fromId).position.y ==
                graph.getNode(edge.toId).position.y);
}

TEST_CASE("Every shipped level gives the standard actor somewhere to walk", "[NavigationGraphBuilder][Level]")
{
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        TileMap tileMap(entry.path().string(), shippedPalettes());
        INFO("level " << entry.path().filename().string() << " has no walkable graph");
        REQUIRE_FALSE(buildNavigationGraph(tileMap, standardProfile()).getEdges().empty());
    }
}
