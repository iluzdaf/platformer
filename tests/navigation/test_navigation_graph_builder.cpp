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
#include "navigation/navigation_graph_steps.hpp"
#include <glaze/glaze.hpp>
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "game/game_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "test_helpers/asset_path.hpp"
#include "tile_map/tile_palette_data.hpp"

namespace
{
    constexpr int FloorRow = 6;
    constexpr int CeilingRow = 4;
    constexpr int MapWidthTiles = 10;

    NavigationProfile profileOfHeight(float height)
    {
        NavigationProfile profile;
        profile.physicsBodyData.colliderSize = glm::vec2(8.0f, height);
        return profile;
    }

    NavigationProfile standardProfile()
    {
        return profileOfHeight(13.0f);
    }

    using Placed = std::vector<std::pair<glm::ivec2, int>>;

    void layRow(Placed &laid, int row, int fromX, int toX)
    {
        for (int x = fromX; x <= toX; ++x)
            laid.push_back({glm::ivec2(x, row), 1});
    }

    Placed floorTiles()
    {
        Placed laid;
        layRow(laid, FloorRow, 0, MapWidthTiles - 1);
        return laid;
    }

    TileMap setupFloor()
    {
        return setupTileMapWith(floorTiles());
    }

    TileMap setupFloorUnderOneTileOfHeadroom()
    {
        Placed laid = floorTiles();
        layRow(laid, CeilingRow, 0, MapWidthTiles - 1);
        return setupTileMapWith(laid);
    }

    constexpr int HighCeilingRow = 2;
    constexpr int PinchColumn = 5;

    TileMap setupCorridorThatPinches()
    {
        Placed laid = floorTiles();
        layRow(laid, HighCeilingRow, 0, MapWidthTiles - 1);
        laid.push_back({glm::ivec2(PinchColumn, FloorRow - 2), 1});
        return setupTileMapWith(laid);
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

    void layFloor(Placed &laid, int groundY, int fromX, int toX)
    {
        layRow(laid, groundY, fromX, toX);
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
    Placed laid;
    layFloor(laid, 5, 2, 4);
    TileMap tileMap = setupTileMapWith(laid);

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
    layFloor(laid, 5, 0, 2);
    layFloor(laid, 5, 6, 9);
    TileMap tileMap = setupTileMapWith(laid);

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
    layFloor(laid, 5, 0, 2);
    layFloor(laid, 5, 6, 9);
    TileMap tileMap = setupTileMapWith(laid);
    NavigationGraph graph = buildNavigationGraph(tileMap, standardProfile());

    std::vector<std::vector<int>> runs = navigation::walkRuns(graph, tileMap, 1);

    REQUIRE(runs.size() == 2);
    for (const std::vector<int> &run : runs)
        for (size_t at = 1; at < run.size(); ++at)
            REQUIRE(graph.getNode(run[at - 1]).position.x < graph.getNode(run[at]).position.x);
}

TEST_CASE("No walk edge passes through a blocked tile", "[NavigationGraphBuilder]")
{
    Placed laid;
    layFloor(laid, 5, 0, 5);
    laid.push_back({glm::ivec2(3, 4), 1});
    TileMap tileMap = setupTileMapWith(laid);

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
    Placed laid;
    layFloor(laid, 5, 0, 3);
    layFloor(laid, 8, 0, 3);
    TileMap tileMap = setupTileMapWith(laid);

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
    float tileSize = static_cast<float>(setupTileMap().getTileSize());

    SECTION("Single Tile Platform at left side of TileMap")
    {
        TileMap tileMap = setupTileMapWith({{{0, 9}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform at right side of TileMap")
    {
        TileMap tileMap = setupTileMapWith({{{9, 9}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({9 * tileSize + tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform where both sides are cliffs")
    {
        TileMap tileMap = setupTileMapWith({{{1, 9}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 9 * tileSize}));
    }

    SECTION("Single Tile Platform where left side is a cliff and right side is a wall")
    {
        TileMap tileMap = setupTileMapWith({{{2, 0}, 1}, {{2, 1}, 1}, {{1, 1}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("Single Tile Platform where right side is a cliff and left side is a wall")
    {
        TileMap tileMap = setupTileMapWith({{{0, 0}, 1}, {{0, 1}, 1}, {{1, 1}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("Single Tile Platform where both sides are walls")
    {
        TileMap tileMap =
            setupTileMapWith({{{0, 0}, 1}, {{0, 1}, 1}, {{1, 1}, 1}, {{2, 0}, 1}, {{2, 1}, 1}});
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 1);
        REQUIRE(navigationGraph.hasNodeAtPosition({1 * tileSize + tileSize / 2, 1 * tileSize}));
    }

    SECTION("2 Tile Platforms")
    {
        Placed laid;
        layFloor(laid, 1, 0, 1);
        TileMap tileMap = setupTileMapWith(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({2 * tileSize, 1 * tileSize}));
    }

    SECTION("3 Tile Platforms")
    {
        Placed laid;
        layFloor(laid, 1, 0, 2);
        TileMap tileMap = setupTileMapWith(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({3 * tileSize, 1 * tileSize}));
    }

    SECTION("5 Tile Platforms")
    {
        Placed laid;
        layFloor(laid, 1, 0, 4);
        TileMap tileMap = setupTileMapWith(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({5 * tileSize, 1 * tileSize}));
    }

    SECTION("10 Tile Platforms")
    {
        Placed laid;
        layFloor(laid, 1, 0, 9);
        TileMap tileMap = setupTileMapWith(laid);
        NavigationGraph navigationGraph = buildNavigationGraph(tileMap, standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 2);
        REQUIRE(navigationGraph.hasNodeAtPosition({0, 1 * tileSize}));
        REQUIRE(navigationGraph.hasNodeAtPosition({10 * tileSize, 1 * tileSize}));
    }

    SECTION("No nodes")
    {
        NavigationGraph navigationGraph = buildNavigationGraph(setupTileMap(), standardProfile());
        REQUIRE(navigationGraph.getNodes().size() == 0);
    }

    SECTION("No walkable tiles")
    {
        Placed laid;
        layFloor(laid, 0, 0, 2);
        TileMap tileMap = setupTileMapWith(laid);
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

    Placed twoPlatforms(int gapTiles, int rowsUp = 0, int widthTiles = 20)
    {
        Placed laid;
        layFloor(laid, PlatformRow, 0, LeftPlatformEnd);
        layFloor(laid, PlatformRow - rowsUp, LeftPlatformEnd + gapTiles + 1, widthTiles - 1);
        return laid;
    }

    TileMap setupTwoPlatforms(int gapTiles, int rowsUp = 0, int widthTiles = 20)
    {
        return setupTileMapWith(
            twoPlatforms(gapTiles, rowsUp, widthTiles), widthTiles, WideMapHeightTiles);
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
    Placed laid = twoPlatforms(GapTiles);
    layFloor(laid, PlatformRow - 2, LeftPlatformEnd + 1, LeftPlatformEnd + GapTiles);
    TileMap tileMap = setupTileMapWith(laid, 20, WideMapHeightTiles);

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
        if (node.kind == NodeKind::OnWall)
            continue;

        INFO("node " << id << " at " << node.position.x << "," << node.position.y);
        REQUIRE(std::fmod(node.position.y, tileSize) == 0.0f);

        glm::ivec2 under = tileMap.tileContaining(node.position + glm::vec2(0.0f, 1.0f));
        glm::ivec2 justBehind = tileMap.tileContaining(node.position + glm::vec2(-1.0f, 1.0f));
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
        Placed laid;
        layFloor(laid, FloorBelowRow, 0, 19);
        layFloor(laid, PlatformRow, 0, LeftPlatformEnd);
        TileMap tileMap = setupTileMapWith(laid, 20, WideMapHeightTiles);

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

    TilePaletteData paletteWithSpikes()
    {
        TilePaletteData palette = getDefaultTileDataMap();
        TileData spikes;
        spikes.deadly = true;
        palette.tiles[SpikeTileIndex] = spikes;
        return palette;
    }

    TileMap setupLedgeAboveSpikes()
    {
        Placed laid;
        for (int x = 0; x < 20; ++x)
            laid.push_back({glm::ivec2(x, FloorBelowRow), SpikeTileIndex});
        layFloor(laid, PlatformRow, 0, LeftPlatformEnd);

        return setupTileMapWith(laid, 20, WideMapHeightTiles, 16, paletteWithSpikes());
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
        Placed laid;
        layFloor(laid, DeepFloorRow, 0, 19);
        layFloor(laid, PlatformRow, 0, NearLedgeEnd);
        layFloor(laid, PlatformRow, FarLedgeStart, FarLedgeEnd);

        return setupTileMapWith(laid, 20, TallMapHeightTiles);
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

TEST_CASE(
    "Nothing falls through spikes to the floor beneath them",
    "[NavigationGraphBuilder][Fall]")
{
    Placed laid;
    layFloor(laid, FloorBelowRow, 0, 19);
    for (int x = 0; x < 20; ++x)
        laid.push_back({glm::ivec2(x, PlatformRow + 1), SpikeTileIndex});
    layFloor(laid, PlatformRow, 0, LeftPlatformEnd);
    TileMap tileMap = setupTileMapWith(laid, 20, WideMapHeightTiles, 16, paletteWithSpikes());

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
    constexpr int ClimbHangRow = ClimbWallTopRow + 1;

    NavigationProfile climberProfile()
    {
        NavigationProfile profile = profileOfHeight(13.0f);
        ActorMotionData motionData;
        motionData.wallHangAbilityData = WallHangAbilityData();
        motionData.wallClimbAbilityData = WallClimbAbilityData();
        profile.motionData = motionData;
        return profile;
    }

    Placed wallFromTheFloor()
    {
        Placed laid;
        layFloor(laid, ClimbFloorRow, 0, 9);
        for (int y = ClimbWallTopRow; y < ClimbFloorRow; ++y)
            laid.push_back({glm::ivec2(ClimbWallX, y), 1});
        return laid;
    }

    TileMap setupWallFromTheFloor()
    {
        return setupTileMapWith(wallFromTheFloor(), 10, 12);
    }

    std::vector<NavigationNode> nodesOnWalls(const NavigationGraph &graph)
    {
        std::vector<NavigationNode> onWalls;
        for (const auto &[id, node] : graph.getNodes())
            if (node.kind == NodeKind::OnWall)
                onWalls.push_back(node);
        return onWalls;
    }

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
                    tileMap.tileContaining(graph.getNode(edge.fromId).position + underfoot).y;
                int to = tileMap.tileContaining(graph.getNode(edge.toId).position + underfoot).y;
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

    REQUIRE(joined.contains({ClimbFloorRow, ClimbHangRow}));
    REQUIRE(joined.contains({ClimbHangRow, ClimbFloorRow}));
    REQUIRE(joined.contains({ClimbHangRow, ClimbWallTopRow}));
    REQUIRE(joined.contains({ClimbWallTopRow, ClimbHangRow}));
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
    hangsOnly.motionData.wallClimbAbilityData.reset();

    REQUIRE(rowsJoinedByClimbing(buildNavigationGraph(tileMap, hangsOnly), tileMap).empty());
}

TEST_CASE(
    "A wall that does not reach the floor is not climbed from it",
    "[NavigationGraphBuilder][Climb]")
{
    Placed laid = wallFromTheFloor();
    laid.push_back({glm::ivec2(ClimbWallX, ClimbFloorRow - 1), 0});
    TileMap tileMap = setupTileMapWith(laid, 10, 12);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbHangRow}));
}

TEST_CASE(
    "A ledge beside the wall breaks one long climb into two",
    "[NavigationGraphBuilder][Climb]")
{
    Placed laid = wallFromTheFloor();
    laid.push_back({glm::ivec2(ClimbWallX - 1, 6), 1});
    laid.push_back({glm::ivec2(ClimbWallX + 1, 6), 1});
    TileMap tileMap = setupTileMapWith(laid, 10, 12);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbFloorRow, ClimbHangRow}));
    REQUIRE(joined.contains({6, ClimbHangRow}));
}

TEST_CASE("A wall you cannot stand on top of is not climbed", "[NavigationGraphBuilder][Climb]")
{
    Placed laid = wallFromTheFloor();
    layFloor(laid, ClimbWallTopRow - 1, 0, 9);
    TileMap tileMap = setupTileMapWith(laid, 10, 12);

    std::set<std::pair<int, int>> joined =
        rowsJoinedByClimbing(buildNavigationGraph(tileMap, climberProfile()), tileMap);

    REQUIRE_FALSE(joined.contains({ClimbHangRow, ClimbWallTopRow}));
}

TEST_CASE("A wall an actor can climb gets a node on it", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();

    std::vector<NavigationNode> onWalls =
        nodesOnWalls(buildNavigationGraph(tileMap, climberProfile()));

    REQUIRE_FALSE(onWalls.empty());
    for (const NavigationNode &node : onWalls)
    {
        glm::vec2 underfoot(0.0f, 1.0f);
        REQUIRE(tileMap.tileContaining(node.position + underfoot).y == ClimbHangRow);
    }
}

TEST_CASE("An actor that cannot climb gets no wall nodes", "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();

    REQUIRE(nodesOnWalls(buildNavigationGraph(tileMap, standardProfile())).empty());
}

TEST_CASE(
    "Climbing goes by way of the wall, never straight onto the ledge",
    "[NavigationGraphBuilder][Climb]")
{
    TileMap tileMap = setupWallFromTheFloor();
    NavigationGraph graph = buildNavigationGraph(tileMap, climberProfile());

    int climbs = 0;
    for (const auto &[id, node] : graph.getNodes())
        for (const NavigationEdge &edge : graph.getOutgoingEdges(id))
        {
            if (edge.type != EdgeType::Climb)
                continue;

            climbs++;
            REQUIRE(
                (graph.getNode(edge.fromId).kind == NodeKind::OnWall ||
                 graph.getNode(edge.toId).kind == NodeKind::OnWall));
        }

    REQUIRE(climbs > 0);
}

TEST_CASE(
    "No level leaves a wall node with nothing joined to it",
    "[NavigationGraphBuilder][Climb]")
{
    GameData gameData = loadGameData();
    NavigationProfile player = buildNavigationProfile(gameData.playerData.actorData);

    for (const std::filesystem::directory_entry &entry :
         std::filesystem::directory_iterator(assetPath("levels")))
    {
        TileMap tileMap = tilesOfLevel(entry.path().string());
        NavigationGraph graph = buildNavigationGraph(tileMap, player);

        for (const auto &[id, node] : graph.getNodes())
        {
            if (node.kind != NodeKind::OnWall)
                continue;

            INFO(entry.path().filename().string() << " node " << id);
            REQUIRE_FALSE(graph.getOutgoingEdges(id).empty());
        }
    }
}

TEST_CASE("A wall an actor cannot grip is not climbed", "[NavigationGraphBuilder][Climb]")
{
    TilePaletteData palette = getDefaultTileDataMap();
    TileData ungrippable;
    ungrippable.solid = true;
    ungrippable.grippable = false;
    palette.tiles[2] = ungrippable;

    Placed laid;
    layFloor(laid, ClimbFloorRow, 0, 9);
    for (int y = ClimbWallTopRow; y < ClimbFloorRow; ++y)
        laid.push_back({glm::ivec2(ClimbWallX, y), 2});

    TileMap ungrippableWall = setupTileMapWith(laid, 10, 12, 16, palette);

    REQUIRE(nodesOnWalls(buildNavigationGraph(ungrippableWall, climberProfile())).empty());
}

TEST_CASE("A jump still falling after ten seconds is no jump", "[NavigationGraphBuilder][Jump]")
{
    constexpr int TallerThanTenSecondsOfFalling = 420;
    Placed laid;
    layFloor(laid, 1, 0, LeftPlatformEnd);
    TileMap tileMap = setupTileMapWith(laid, 20, TallerThanTenSecondsOfFalling);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE(
    "The easiest jump is kept whichever order the arcs are tried in",
    "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupTwoPlatforms(2);
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

TEST_CASE("A place is not walkable to itself, nor to another row", "[NavigationGraphBuilder]")
{
    Placed laid;
    layFloor(laid, 5, 0, 9);
    layFloor(laid, 3, 0, 9);
    TileMap tileMap = setupTileMapWith(laid);
    glm::vec2 onTheLowerFloor = tileMap.feetOnTile(glm::ivec2(2, 4));
    glm::vec2 onTheUpperFloor = tileMap.feetOnTile(glm::ivec2(5, 2));

    REQUIRE_FALSE(navigation::isWalkableBetween(tileMap, onTheLowerFloor, onTheLowerFloor, 1));
    REQUIRE_FALSE(navigation::isWalkableBetween(tileMap, onTheLowerFloor, onTheUpperFloor, 1));
}

TEST_CASE(
    "A fall onto a floor the graph has no node for is no edge",
    "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = setupLedgeAboveFloor();
    NavigationGraph graph;
    graph.addNode(0, takeOffPosition(tileMap));

    navigation::addFallEdges(graph, tileMap, jumperProfile(), 1);

    REQUIRE(graph.getEdges().empty());
}

TEST_CASE(
    "A jump onto a floor the graph has no node for is no edge",
    "[NavigationGraphBuilder][Jump]")
{
    TileMap tileMap = setupLedgeAboveFloor();
    NavigationGraph graph;
    graph.addNode(0, takeOffPosition(tileMap));
    glm::vec2 comesDown(takeOffPosition(tileMap).x + 40.0f, static_cast<float>(FloorBelowRow * 16));
    std::vector<navigation::ChosenJump> jumps{{0, {takeOffPosition(tileMap), comesDown}, 0.2f}};

    navigation::addJumpEdges(graph, tileMap, 1, jumps);

    REQUIRE(graph.getEdges().empty());
}
