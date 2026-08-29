#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <set>
#include <vector>
#include "navigation/navigation_edge.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/gravity_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "actor/abilities/wall_hang_ability_data.hpp"
#include "actor/abilities/wall_climb_ability_data.hpp"
#include <utility>
#include "navigation/navigation_graph_builder.hpp"
#include <glaze/glaze.hpp>
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "game/game_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_kind.hpp"
#include "tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"
#include "tile_map/tile_palette.hpp"

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
        REQUIRE(graph.getNode(edge.fromId).position.y == graph.getNode(edge.toId).position.y);
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

    NavigationProfile profileThatMoves(float height, const ActorMotionData &motionData)
    {
        NavigationProfile profile = profileOfHeight(height);
        profile.jumpArcs = simulateJumpArcs(motionData);
        profile.motionData = motionData;
        return profile;
    }

    ActorMotionData fallerMotionData()
    {
        ActorMotionData motionData;
        motionData.gravityAbilityData = GravityAbilityData{};
        return motionData;
    }

    NavigationProfile jumperProfile()
    {
        return profileThatMoves(13.0f, jumperMotionData());
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

    bool jumpsAcrossTo(const NavigationGraph &graph, glm::vec2 from, glm::vec2 farSide)
    {
        constexpr float Tolerance = 0.5f;
        for (const auto &edge : graph.getEdges())
        {
            if (edge.type != EdgeType::Jump)
                continue;

            if (glm::distance(graph.getNode(edge.fromId).position, from) > Tolerance)
                continue;

            NavigationNode to = graph.getNode(edge.toId);
            if (std::abs(to.position.y - farSide.y) > Tolerance)
                continue;

            bool overThere = farSide.x > from.x ? to.position.x >= farSide.x - Tolerance
                                                : to.position.x <= farSide.x + Tolerance;
            if (overThere)
                return true;
        }

        return false;
    }

    const NavigationEdge *onlyJumpFrom(const NavigationGraph &graph, int fromId)
    {
        const NavigationEdge *only = nullptr;
        for (const auto &edge : graph.getOutgoingEdges(fromId))
            if (edge.type == EdgeType::Jump)
            {
                if (only)
                    return nullptr;
                only = &edge;
            }

        return only;
    }

    bool hasEdgeBetween(const NavigationGraph &graph, glm::vec2 from, glm::vec2 to, EdgeType type)
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

    REQUIRE(jumpsAcrossTo(graph, takeOffPosition(tileMap), landingPosition(tileMap, GapTiles)));
}

TEST_CASE("A jumper crosses a gap in both directions", "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(jumpsAcrossTo(graph, landingPosition(tileMap, GapTiles), takeOffPosition(tileMap)));
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

    REQUIRE(
        jumpsAcrossTo(graph, takeOffPosition(tileMap), landingPosition(tileMap, GapTiles, RowsUp)));
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

TEST_CASE(
    "A ceiling over the gap blocks a jump that would otherwise clear it",
    "[NavigationGraphBuilder][Jump]")
{
    constexpr int GapTiles = 3;
    TileMap tileMap = setupTwoPlatforms(GapTiles);
    for (int x = LeftPlatformEnd + 1; x <= LeftPlatformEnd + GapTiles; ++x)
        tileMap.setTileIndex(glm::ivec2(x, PlatformRow - 2), 1);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE_FALSE(hasEdgeBetween(
        graph, takeOffPosition(tileMap), landingPosition(tileMap, GapTiles), EdgeType::Jump));
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

TEST_CASE(
    "The shipped explorer can cross the gap in level6",
    "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData = loadGameData();

    NavigationProfile explorer = buildNavigationProfile(gameData.npcData.at("explorer").actorData);
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph = buildNavigationGraph(tileMap, explorer);

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) > 0);
}

TEST_CASE(
    "The shipped explorer can get up to level6's top platform and back",
    "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData = loadGameData();

    NavigationProfile explorer = buildNavigationProfile(gameData.npcData.at("explorer").actorData);
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph = buildNavigationGraph(tileMap, explorer);

    int topPlatformId = -1;
    for (const auto &[id, node] : graph.getNodes())
        if (node.position.y < 100.0f)
            topPlatformId = id;

    REQUIRE(topPlatformId >= 0);

    std::vector<int> reachable = roundTripFrom(graph, topPlatformId);
    REQUIRE(reachable.size() > 2);
}

TEST_CASE(
    "The shipped actors can reach every surface in level6",
    "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData = loadGameData();
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    auto reachesEverySurface = [&](const ActorData &actorData)
    {
        NavigationGraph graph = buildNavigationGraph(tileMap, buildNavigationProfile(actorData));

        std::set<float> surfaces;
        for (const auto &[id, node] : graph.getNodes())
            surfaces.insert(node.position.y);

        for (const auto &[id, node] : graph.getNodes())
        {
            std::set<float> fromHere;
            for (int to : roundTripFrom(graph, id))
                fromHere.insert(graph.getNode(to).position.y);
            if (fromHere == surfaces)
                return true;
        }
        return false;
    };

    REQUIRE(reachesEverySurface(gameData.npcData.at("explorer").actorData));
    REQUIRE(reachesEverySurface(gameData.playerData.actorData));
}

TEST_CASE(
    "A way up does not depend on something having fallen there",
    "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData = loadGameData();
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph = buildNavigationGraph(
        tileMap, buildNavigationProfile(gameData.npcData.at("explorer").actorData));

    float floorY = 192.0f;
    bool getsOffTheFloor = false;
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        NavigationNode from = graph.getNode(edge.fromId);
        INFO(
            "jump from node " << edge.fromId << " at " << from.position.x << ","
                              << from.position.y);
        REQUIRE(from.kind == NodeKind::OnFoot);

        if (std::abs(from.position.y - floorY) < 0.5f &&
            graph.getNode(edge.toId).position.y < floorY)
            getsOffTheFloor = true;
    }

    REQUIRE(getsOffTheFloor);
}

TEST_CASE(
    "The shipped player is offered every climb level6 asks of it",
    "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData = loadGameData();
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph =
        buildNavigationGraph(tileMap, buildNavigationProfile(gameData.playerData.actorData));

    for (auto [from, to] :
         {std::pair(192.0f, 160.0f), std::pair(160.0f, 128.0f), std::pair(128.0f, 96.0f)})
    {
        bool offered = false;
        for (const auto &edge : graph.getEdges())
            if (edge.type == EdgeType::Jump &&
                std::abs(graph.getNode(edge.fromId).position.y - from) < 0.5f &&
                std::abs(graph.getNode(edge.toId).position.y - to) < 0.5f)
                offered = true;

        INFO("no jump from y " << from << " up to y " << to);
        REQUIRE(offered);
    }
}

TEST_CASE("Every node stands on the top of a tile", "[NavigationGraphBuilder][Level]")
{
    GameData gameData = loadGameData();
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    NavigationGraph graph = buildNavigationGraph(
        tileMap, buildNavigationProfile(gameData.npcData.at("explorer").actorData));

    float tileSize = static_cast<float>(tileMap.getTileSize());
    for (const auto &[id, node] : graph.getNodes())
    {
        INFO("node " << id << " at " << node.position.x << "," << node.position.y);
        REQUIRE(std::fmod(node.position.y, tileSize) == 0.0f);

        glm::ivec2 under = tileMap.worldToTilePosition(node.position + glm::vec2(0.0f, 1.0f));
        glm::ivec2 justBehind = tileMap.worldToTilePosition(node.position + glm::vec2(-1.0f, 1.0f));
        REQUIRE(
            (tileMap.getTileAtTilePosition(under).isSolid() ||
             tileMap.getTileAtTilePosition(justBehind).isSolid()));
    }
}

TEST_CASE(
    "The shipped villager is offered no jumps at all",
    "[NavigationGraphBuilder][Jump][Level]")
{
    GameData gameData = loadGameData();

    NavigationProfile villager = buildNavigationProfile(gameData.npcData.at("villager").actorData);
    TileMap tileMap = tilesOfLevel(assetPath("levels/level6.json"));

    REQUIRE(villager.jumpArcs.empty());
    REQUIRE(countEdgesOfType(buildNavigationGraph(tileMap, villager), EdgeType::Jump) == 0);
}

TEST_CASE("A jump edge records the hold that made it", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Jump)
            REQUIRE(edge.holdDuration > 0.0f);
}

TEST_CASE("A walk edge is held for nothing", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(3);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Walk)
            REQUIRE(edge.holdDuration == 0.0f);
}

TEST_CASE("A gap needing less than a full jump records less", "[NavigationGraphBuilder][Jump]")
{
    NavigationProfile profile = jumperProfile();
    TileMap tileMap = setupTwoPlatforms(1);

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Jump)
            REQUIRE(edge.holdDuration <= profile.jumpArcs.front().holdDuration);
}

namespace
{
    constexpr int FloorBelowRow = PlatformRow + 3;

    TileMap setupLedgeAboveFloor()
    {
        TileMap tileMap = setupTileMap(20, WideMapHeightTiles);
        for (int x = 0; x < 20; ++x)
            tileMap.setTileIndex(glm::ivec2(x, FloorBelowRow), 1);

        for (int x = 0; x <= LeftPlatformEnd; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        return tileMap;
    }
}

TEST_CASE("Walking off a ledge is an edge to the floor below", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) > 0);
}

TEST_CASE("A fall only ever goes down", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Fall)
            REQUIRE(graph.getNode(edge.toId).position.y > graph.getNode(edge.fromId).position.y);
}

TEST_CASE("Falling is not offered where you could walk", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
}

TEST_CASE("A profile that cannot move still falls", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();
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
    TileMap tileMap = setupLedgeAboveFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) > 0);
}

TEST_CASE("A fall is drawn as the straight drop it is", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Fall)
            REQUIRE(edge.path.empty());
}

TEST_CASE("A node falls to the one below it and nowhere else", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();

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

namespace
{
    constexpr int SpikeTileIndex = 2;

    TilePalette paletteWithSpikes()
    {
        TilePalette palette = getDefaultTileDataMap();
        palette[SpikeTileIndex] = TileData{
            TileKind::Spikes,
            std::nullopt,
            std::nullopt,
            std::nullopt,
            glm::vec2(0.0f),
            glm::vec2(16.0f)};
        return palette;
    }

    TileMap setupLedgeAboveSpikes()
    {
        TileMap tileMap = setupTileMap(20, WideMapHeightTiles, 16, paletteWithSpikes());
        for (int x = 0; x < 20; ++x)
            tileMap.setTileIndex(glm::ivec2(x, FloorBelowRow), SpikeTileIndex);

        for (int x = 0; x <= LeftPlatformEnd; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        return tileMap;
    }
    int nodeJustPastTheLedge(const NavigationGraph &graph, float floorY)
    {
        float ledgeEdgeX = static_cast<float>(LeftPlatformEnd + 1) * 16.0f;
        for (const auto &[id, node] : graph.getNodes())
            if (std::abs(node.position.y - floorY) < 0.5f && node.position.x > ledgeEdgeX &&
                node.position.x < ledgeEdgeX + 8.0f)
                return id;

        return -1;
    }
    constexpr int TallMapHeightTiles = 18;
    constexpr int DeepFloorRow = 16;
    constexpr int NearLedgeEnd = 2;
    constexpr int FarLedgeStart = 6;
    constexpr int FarLedgeEnd = 12;

    TileMap setupLedgesAboveFloor()
    {
        TileMap tileMap = setupTileMap(20, TallMapHeightTiles);
        for (int x = 0; x < 20; ++x)
            tileMap.setTileIndex(glm::ivec2(x, DeepFloorRow), 1);

        for (int x = 0; x <= NearLedgeEnd; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        for (int x = FarLedgeStart; x <= FarLedgeEnd; ++x)
            tileMap.setTileIndex(glm::ivec2(x, PlatformRow), 1);

        return tileMap;
    }

    int nodeAt(const NavigationGraph &graph, glm::vec2 position)
    {
        for (const auto &[id, node] : graph.getNodes())
            if (glm::distance(node.position, position) < 0.5f)
                return id;

        return -1;
    }
}

TEST_CASE("A jump is the smallest one that reaches", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupLedgesAboveFloor();
    float ledgeY = static_cast<float>(PlatformRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int acrossTheGap =
        nodeAt(graph, glm::vec2(static_cast<float>(NearLedgeEnd + 1) * 16.0f, ledgeY));
    int farSide = nodeAt(graph, glm::vec2(static_cast<float>(FarLedgeStart) * 16.0f, ledgeY));
    int longWayOff = nodeAt(graph, glm::vec2(0.0f, ledgeY));
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
    TileMap tileMap = setupLedgesAboveFloor();
    float ledgeY = static_cast<float>(PlatformRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int takeOffId = nodeAt(graph, glm::vec2(0.0f, ledgeY));
    int nearEdgeId = nodeAt(graph, glm::vec2(static_cast<float>(FarLedgeStart) * 16.0f, ledgeY));
    REQUIRE(takeOffId >= 0);
    REQUIRE(nearEdgeId >= 0);
    REQUIRE(nodeAt(graph, glm::vec2(static_cast<float>(FarLedgeEnd + 1) * 16.0f, ledgeY)) >= 0);

    const NavigationEdge *only = onlyJumpFrom(graph, takeOffId);
    REQUIRE(only);
    REQUIRE(graph.getNode(only->toId).position.y == ledgeY);
    REQUIRE(
        graph.getNode(only->toId).position.x >= static_cast<float>(FarLedgeStart) * 16.0f - 0.5f);
}

TEST_CASE("Nothing jumps to where it could walk", "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupLedgesAboveFloor();
    float floorY = static_cast<float>(DeepFloorRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    size_t onTheFloor = 0;
    for (const auto &[id, node] : graph.getNodes())
        if (std::abs(node.position.y - floorY) < 0.5f)
            ++onTheFloor;

    REQUIRE(onTheFloor >= 4);

    int jumpsAlongTheFloor = 0;
    for (const auto &edge : graph.getEdges())
    {
        if (edge.type != EdgeType::Jump)
            continue;

        if (std::abs(graph.getNode(edge.fromId).position.y - floorY) < 0.5f &&
            std::abs(graph.getNode(edge.toId).position.y - floorY) < 0.5f)
            ++jumpsAlongTheFloor;
    }

    REQUIRE(jumpsAlongTheFloor == 0);
}

TEST_CASE("Nothing falls onto spikes", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveSpikes();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
}

TEST_CASE("A ledge gets a node directly below it", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();
    float floorY = static_cast<float>(FloorBelowRow) * 16.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(nodeJustPastTheLedge(graph, floorY) >= 0);
}

TEST_CASE("The fall from a ledge goes to the node below it", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();
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
    TileMap tileMap = setupLedgeAboveFloor();
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
    TileMap tileMap = setupLedgeAboveFloor();

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

namespace
{
    constexpr int ClimbFloorRow = 8;
    constexpr int ClimbWallX = 5;
    constexpr int ClimbWallTopRow = 4;

    NavigationProfile climberProfile()
    {
        NavigationProfile profile = profileOfHeight(13.0f);
        ActorMotionData motionData;
        motionData.wallHangAbilityData = WallHangAbilityData();
        motionData.wallClimbAbilityData = WallClimbAbilityData();
        profile.motionData = motionData;
        return profile;
    }

    // A floor at row 8, and a wall rising from it to row 4 whose top can be stood on.
    TileMap setupWallFromTheFloor()
    {
        TileMap tileMap = setupTileMap(10, 12);
        for (int x = 0; x < 10; ++x)
            tileMap.setTileIndex(glm::ivec2(x, ClimbFloorRow), 1);
        for (int y = ClimbWallTopRow; y < ClimbFloorRow; ++y)
            tileMap.setTileIndex(glm::ivec2(ClimbWallX, y), 1);
        return tileMap;
    }

    // Which surfaces climb edges join, named by the tile row each surface sits on.
    std::set<std::pair<int, int>> rowsJoinedByClimbing(
        const NavigationGraph &graph,
        const TileMap &tileMap)
    {
        std::set<std::pair<int, int>> joined;
        for (const auto &[id, node] : graph.getNodes())
            for (const NavigationEdge &edge : graph.getOutgoingEdges(id))
            {
                if (edge.type != EdgeType::Climb)
                    continue;

                glm::vec2 underfoot(0.0f, 1.0f);
                int from =
                    tileMap.worldToTilePosition(graph.getNode(edge.fromId).position + underfoot).y;
                int to =
                    tileMap.worldToTilePosition(graph.getNode(edge.toId).position + underfoot).y;
                joined.insert({from, to});
            }
        return joined;
    }
}

TEST_CASE(
    "A wall an actor can hang on becomes a way up and back down",
    "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE(joined.contains({ClimbFloorRow, ClimbWallTopRow}));
    REQUIRE(joined.contains({ClimbWallTopRow, ClimbFloorRow}));
}

TEST_CASE("An actor that cannot climb is given no way up a wall", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();

    REQUIRE(
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, standardProfile()), tileMap).empty());
}

TEST_CASE("Hanging without climbing is no way up", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();
    NavigationProfile hangsOnly = climberProfile();
    hangsOnly.motionData->wallClimbAbilityData.reset();

    REQUIRE(rowsJoinedByClimbing(buildNavigationGraph(tileMap, hangsOnly), tileMap).empty());
}

TEST_CASE(
    "A wall that does not reach the floor is not climbed from it",
    "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();
    tileMap.setTileIndex(glm::ivec2(ClimbWallX, ClimbFloorRow - 1), 0);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbWallTopRow}));
}

TEST_CASE(
    "A ledge beside the wall breaks one long climb into two",
    "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();
    tileMap.setTileIndex(glm::ivec2(ClimbWallX - 1, 6), 1);
    tileMap.setTileIndex(glm::ivec2(ClimbWallX + 1, 6), 1);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbWallTopRow}));
    REQUIRE(joined.contains({6, ClimbWallTopRow}));
}

TEST_CASE("A wall you cannot stand on top of is not climbed", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();
    for (int x = 0; x < 10; ++x)
        tileMap.setTileIndex(glm::ivec2(x, ClimbWallTopRow - 1), 1);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbWallTopRow}));
}

TEST_CASE("A wall with no room beside its top is not climbed", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();
    tileMap.setTileIndex(glm::ivec2(ClimbWallX - 1, ClimbWallTopRow - 1), 1);
    tileMap.setTileIndex(glm::ivec2(ClimbWallX + 1, ClimbWallTopRow - 1), 1);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbWallTopRow}));
}
