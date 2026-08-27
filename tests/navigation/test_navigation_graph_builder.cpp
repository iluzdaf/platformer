#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <filesystem>
#include <set>
#include <vector>
#include "navigation/navigation_graph_builder.hpp"
#include <glaze/glaze.hpp>
#include "navigation/jump_arc.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "game/game_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "tile_map/tile_map.hpp"
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

        TileMap tileMap = tilesOfLevel(entry.path().string());
        INFO("level " << entry.path().filename().string() << " has no walkable graph");
        REQUIRE_FALSE(buildNavigationGraph(tileMap, standardProfile()).getEdges().empty());
    }
}

TEST_CASE("Where a node sits on a platform", "[NavigationGraphBuilder]")
{
    TileMap tileMap = setupTileMap();
    float tileSize = static_cast<float>(tileMap.getTileSize());

    SECTION("Single Tile Platform at left side of TileMap")
    {
        tileMap.setTileIndex(glm::ivec2(0, 9), 1);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform at right side of TileMap")
    {
        tileMap.setTileIndex(glm::ivec2(9, 9), 1);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({9 * tileSize + tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform where both sides are cliffs")
    {
        tileMap.setTileIndex(glm::ivec2(1, 9), 1);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform where left side is a cliff and right side is a wall")
    {
        tileMap.setTileIndex(glm::ivec2(2, 0), 1);
        tileMap.setTileIndex(glm::ivec2(2, 1), 1);
        tileMap.setTileIndex(glm::ivec2(1, 1), 1);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("Single Tile Platform where right side is a cliff and left side is a wall")
    {
        tileMap.setTileIndex(glm::ivec2(0, 0), 1);
        tileMap.setTileIndex(glm::ivec2(0, 1), 1);
        tileMap.setTileIndex(glm::ivec2(1, 1), 1);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("Single Tile Platform where both sides are walls")
    {
        tileMap.setTileIndex(glm::ivec2(0, 0), 1);
        tileMap.setTileIndex(glm::ivec2(0, 1), 1);
        tileMap.setTileIndex(glm::ivec2(1, 1), 1);
        tileMap.setTileIndex(glm::ivec2(2, 0), 1);
        tileMap.setTileIndex(glm::ivec2(2, 1), 1);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("2 Tile Platforms")
    {
        for (int x = 0; x < 2; ++x)
        {
            tileMap.setTileIndex(glm::ivec2(x, 1), 1);
        }
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({2 * tileSize, 1 * tileSize}));
    }

    SECTION("3 Tile Platforms")
    {
        for (int x = 0; x < 3; ++x)
        {
            tileMap.setTileIndex(glm::ivec2(x, 1), 1);
        }
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({3 * tileSize, 1 * tileSize}));
    }

    SECTION("5 Tile Platforms")
    {
        for (int x = 0; x < 5; ++x)
        {
            tileMap.setTileIndex(glm::ivec2(x, 1), 1);
        }
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({5 * tileSize, 1 * tileSize}));
    }

    SECTION("10 Tile Platforms")
    {
        for (int x = 0; x < 10; ++x)
        {
            tileMap.setTileIndex(glm::ivec2(x, 1), 1);
        }
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({10 * tileSize, 1 * tileSize}));
    }

    SECTION("No nodes")
    {
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 0);
    }

    SECTION("No walkable tiles")
    {
        for (int x = 0; x < 3; ++x)
        {
            tileMap.setTileIndex(glm::ivec2(x, 0), 1);
        }
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 0);
    }
}

namespace
{
    constexpr int PlatformRow = 8;
    constexpr int LeftPlatformEnd = 4;
    constexpr int WideMapHeightTiles = 12;

    ActorMotionData jumperMotionData()
    {
        ActorMotionData motionData;
        motionData.moveAbilityData = MoveAbilityData{};
        motionData.gravityAbilityData = GravityAbilityData{};
        motionData.jumpAbilityData = JumpAbilityData{};
        return motionData;
    }

    NavigationProfile jumperProfile()
    {
        NavigationProfile profile = standardProfile();
        profile.jumpArcs = simulateJumpArcs(jumperMotionData());
        return profile;
    }

    TileMap setupTwoPlatforms(int gapTiles, int rowsUp = 0, int widthTiles = 20)
    {
        TileMap tileMap = setupTileMap(widthTiles, WideMapHeightTiles);

        for (int x = 0; x <= LeftPlatformEnd; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        for (int x = LeftPlatformEnd + gapTiles + 1; x < widthTiles; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow - rowsUp), 1);

        return tileMap;
    }

    glm::vec2 takeOffPosition(const TileMap &tileMap)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        return glm::vec2(
            static_cast<float>(LeftPlatformEnd + 1) * tileSize,
            static_cast<float>(PlatformRow) * tileSize);
    }

    glm::vec2 landingPosition(const TileMap &tileMap, int gapTiles, int rowsUp = 0)
    {
        float tileSize = static_cast<float>(tileMap.getTileSize());
        return glm::vec2(
            static_cast<float>(LeftPlatformEnd + gapTiles + 1) * tileSize,
            static_cast<float>(PlatformRow - rowsUp) * tileSize);
    }

    bool hasEdgeBetween(
        const NavigationGraph &graph,
        glm::vec2 from,
        glm::vec2 to,
        EdgeType type)
    {
        constexpr float Tolerance = 0.5f;
        for (const auto &edge : graph.getEdges())
        {
            if (edge.type != type)
                continue;

            if (glm::distance(graph.getNode(edge.fromId).position, from) > Tolerance)
                continue;

            if (glm::distance(graph.getNode(edge.toId).position, to) <= Tolerance)
                return true;
        }

        return false;
    }

    int countEdgesOfType(const NavigationGraph &graph, EdgeType type)
    {
        int count = 0;
        for (const auto &edge : graph.getEdges())
            if (edge.type == type)
                ++count;
        return count;
    }
}

TEST_CASE("A profile that cannot jump gets no jump edges", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE("A jumper crosses a gap it can clear", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(hasEdgeBetween(
        graph,
        takeOffPosition(tileMap),
        landingPosition(tileMap, GapTiles),
        EdgeType::Jump));
}

TEST_CASE("A jumper crosses a gap in both directions", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(hasEdgeBetween(
        graph,
        landingPosition(tileMap, GapTiles),
        takeOffPosition(tileMap),
        EdgeType::Jump));
}

TEST_CASE("A jumper does not cross a gap beyond its reach", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(14, 0, 30);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE("A jumper reaches a ledge two tiles up", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 2;
    constexpr int RowsUp = 2;
    TileMap tileMap = setupTwoPlatforms(GapTiles, RowsUp);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(hasEdgeBetween(
        graph,
        takeOffPosition(tileMap),
        landingPosition(tileMap, GapTiles, RowsUp),
        EdgeType::Jump));
}

TEST_CASE("A jumper does not reach a ledge six tiles up", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 2;
    constexpr int RowsUp = 6;
    TileMap tileMap = setupTwoPlatforms(GapTiles, RowsUp);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE_FALSE(hasEdgeBetween(
        graph,
        takeOffPosition(tileMap),
        landingPosition(tileMap, GapTiles, RowsUp),
        EdgeType::Jump));
}

TEST_CASE("A ceiling over the gap blocks a jump that would otherwise clear it", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);
    for (int x = LeftPlatformEnd + 1; x <= LeftPlatformEnd + GapTiles; ++x)
        tileMap.setTileIndex(glm::ivec2(x, PlatformRow - 2), 1);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE_FALSE(hasEdgeBetween(
        graph,
        takeOffPosition(tileMap),
        landingPosition(tileMap, GapTiles),
        EdgeType::Jump));
}

TEST_CASE("A jumper still walks the platform it stands on", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Walk) > 0);
}

TEST_CASE("A jump edge carries the arc that produced it", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        REQUIRE(edge.path.size() > 2);
        REQUIRE(edge.path.front() == graph.getNode(edge.fromId).position);
        REQUIRE(edge.path.back().y == graph.getNode(edge.toId).position.y);
    }
}

TEST_CASE("An arc on an edge rises above both of its ends", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        float highest = edge.path.front().y;
        for (const glm::vec2 &position : edge.path)
            highest = std::min(highest, position.y);

        REQUIRE(highest < graph.getNode(edge.fromId).position.y);
        REQUIRE(highest < graph.getNode(edge.toId).position.y);
    }
}

TEST_CASE("A walk edge is drawn straight", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Walk)
            REQUIRE(edge.path.empty());
}

TEST_CASE("The shipped explorer can cross the gap in level6", "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));

    NavigationProfile explorer =
        buildNavigationProfile(gameData.npcData.at("explorer").actorData);
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph = buildNavigationGraph(tileMap, explorer);

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) > 0);
}

TEST_CASE("Every jump the explorer is offered is one it can hold out", "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));

    NavigationProfile explorer =
        buildNavigationProfile(gameData.npcData.at("explorer").actorData);
    NavigationProfile holdingThroughout = explorer;
    holdingThroughout.jumpArcs.resize(1);
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    REQUIRE(countEdgesOfType(buildNavigationGraph(tileMap, explorer), EdgeType::Jump) ==
            countEdgesOfType(buildNavigationGraph(tileMap, holdingThroughout), EdgeType::Jump));
}

TEST_CASE("The shipped villager is offered no jumps at all", "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData;
    REQUIRE_FALSE(glz::read_file_json(gameData, assetPath("game_data.json"), std::string{}));

    NavigationProfile villager =
        buildNavigationProfile(gameData.npcData.at("villager").actorData);
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    REQUIRE(villager.jumpArcs.empty());
    REQUIRE(countEdgesOfType(buildNavigationGraph(tileMap, villager), EdgeType::Jump) == 0);
}
