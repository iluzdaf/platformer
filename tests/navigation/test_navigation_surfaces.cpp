#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "helpers/actors.hpp"
#include "helpers/palettes.hpp"
#include "helpers/tiles.hpp"
#include "navigation/jump_simulation.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_map.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "physics/aabb.hpp"

namespace
{
    constexpr int Full = 1;
    constexpr int APixelLower = 2;
    constexpr int InsetFromTheLeft = 3;
    constexpr int InsetFromTheRight = 4;
    constexpr int AThinFloorLowDown = 5;
    constexpr int Floor = 9;
    constexpr float FloorTop = Floor * 16.0f;

    TileData solidWith(std::optional<TileColliderData> collider)
    {
        TileData tile;
        tile.solid = tile.grippable = true;
        tile.collider = collider;
        return tile;
    }

    TileMap aMapOf(const Placed &laid, int width = 10, int height = 10)
    {
        TilePaletteData palette = paletteOf(
            {{EmptyTile, TileData{}},
             {Full, solidWith(std::nullopt)},
             {APixelLower, solidWith(TileColliderData{{0.0f, 1.0f}, {16.0f, 15.0f}})},
             {InsetFromTheLeft, solidWith(TileColliderData{{2.0f, 0.0f}, {14.0f, 16.0f}})},
             {InsetFromTheRight, solidWith(TileColliderData{{0.0f, 0.0f}, {14.0f, 16.0f}})},
             {AThinFloorLowDown, solidWith(TileColliderData{{0.0f, 12.0f}, {16.0f, 4.0f}})}});
        return aTileMap(laid, width, height, 16, palette);
    }

    NavigationProfile steppingNoHigherThan(float stepHeight)
    {
        NavigationProfile profile = standardProfile();
        profile.physicsBodyData.stepHeight = stepHeight;
        return profile;
    }
}

TEST_CASE("A node stands on the top of its tile's collider", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 2, 5, APixelLower);

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid), standardProfile());

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE(graph.hasNodeAtPosition({32.0f, FloorTop + 1.0f}));
    REQUIRE(graph.hasNodeAtPosition({96.0f, FloorTop + 1.0f}));
}

TEST_CASE("A run ends at the edge of its collider, not of its tile", "[NavigationSurfaces]")
{
    Placed laid;
    laid.push_back({{2, Floor}, InsetFromTheLeft});
    layRow(laid, Floor, 3, 5, Full);
    laid.push_back({{6, Floor}, InsetFromTheRight});

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid), standardProfile());

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE(graph.hasNodeAtPosition({34.0f, FloorTop}));
    REQUIRE(graph.hasNodeAtPosition({110.0f, FloorTop}));
}

TEST_CASE("A platform one tile wide stands at the middle of its collider", "[NavigationSurfaces]")
{
    NavigationGraph graph =
        buildNavigationGraph(aMapOf({{{4, Floor}, InsetFromTheLeft}}), standardProfile());

    REQUIRE(graph.getNodes().size() == 1);
    REQUIRE(graph.hasNodeAtPosition({73.0f, FloorTop}));
}

TEST_CASE("A step no higher than the body steps is one run", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 1, 3, Full);
    layRow(laid, Floor, 4, 6, APixelLower);
    TileMap tileMap = aMapOf(laid);
    NavigationProfile profile = standardProfile();

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(graph.getNodes().size() == 2);
    REQUIRE(graph.hasNodeAtPosition({16.0f, FloorTop}));
    REQUIRE(graph.hasNodeAtPosition({112.0f, FloorTop + 1.0f}));
    REQUIRE(
        navigation::walkRuns(graph, tileMap, 1, profile.physicsBodyData.stepHeight).size() == 1);
}

TEST_CASE("A step higher than the body steps is two runs", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 1, 3, Full);
    layRow(laid, Floor, 4, 6, APixelLower);
    TileMap tileMap = aMapOf(laid);

    NavigationGraph graph = buildNavigationGraph(tileMap, steppingNoHigherThan(0.5f));

    REQUIRE(graph.hasNodeAtPosition({64.0f, FloorTop}));
    REQUIRE(graph.hasNodeAtPosition({64.0f, FloorTop + 1.0f}));
    REQUIRE(navigation::walkRuns(graph, tileMap, 1, 0.5f).size() == 2);
}

TEST_CASE("A floor too far down to step onto is walked down to by falling", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 1, 3, Full);
    layRow(laid, Floor, 4, 6, AThinFloorLowDown);
    TileMap tileMap = aMapOf(laid);
    NavigationProfile profile = jumperProfile();

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(
        navigation::walkRuns(graph, tileMap, 1, profile.physicsBodyData.stepHeight).size() == 2);
    std::optional<int> edge = graph.nodeAtPosition({64.0f, FloorTop});
    REQUIRE(edge);
    bool fallsDown = false;
    for (const NavigationEdge &leaving : graph.getOutgoingEdges(*edge))
        fallsDown = fallsDown || (leaving.type == EdgeType::Fall &&
                                  graph.getNode(leaving.toId).feet.y == FloorTop + 12.0f);
    REQUIRE(fallsDown);
}

TEST_CASE("A fall lands on the top of the collider it comes down on", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, 5, 3, 5, Full);
    layRow(laid, Floor, 0, 9, APixelLower);

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid), jumperProfile());

    bool landed = false;
    for (const auto &[id, node] : graph.getNodes())
        if (node.kind == NodeKind::Landing)
        {
            landed = true;
            REQUIRE(node.feet.y == FloorTop + 1.0f);
        }
    REQUIRE(landed);
}

TEST_CASE("A jump comes to rest on the top of the collider it lands on", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 0, 29, APixelLower);
    TileMap tileMap = aMapOf(laid, 30, 12);
    NavigationProfile profile = jumperProfile();

    JumpAttempt attempt = simulateJumpAgainst(
        tileMap,
        profile.abilities,
        profile.physicsBodyData,
        glm::vec2(80.0f, FloorTop + 1.0f),
        1.0f,
        1.0f);

    REQUIRE(attempt.landed);
    REQUIRE(attempt.path.back().y == FloorTop + 1.0f);
}

TEST_CASE(
    "A wall's foot on a lower collider is the end of the run beside it",
    "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 0, 5, APixelLower);
    layColumn(laid, 6, 3, Floor, Full);
    TileMap tileMap = aMapOf(laid);

    NavigationGraph graph = buildNavigationGraph(tileMap, climberProfile());

    REQUIRE_FALSE(graph.hasNodeAtPosition({96.0f, FloorTop}));
    std::optional<int> foot = graph.nodeAtPosition({96.0f, FloorTop + 1.0f});
    REQUIRE(foot);
    bool climbs = false;
    for (const NavigationEdge &leaving : graph.getOutgoingEdges(*foot))
        climbs = climbs || leaving.type == EdgeType::Climb;
    REQUIRE(climbs);
}

TEST_CASE("Feet on a tile stand on the collider beneath it", "[NavigationSurfaces]")
{
    TileMap tileMap = aMapOf({{{3, Floor}, APixelLower}, {{5, Floor}, AThinFloorLowDown}});

    REQUIRE(tileMap.feetOnTile({3, Floor - 1}) == glm::vec2(56.0f, FloorTop + 1.0f));
    REQUIRE(tileMap.feetOnTile({5, Floor - 1}) == glm::vec2(88.0f, FloorTop + 12.0f));
    REQUIRE(tileMap.feetOnTile({4, Floor - 1}) == glm::vec2(72.0f, FloorTop));
}

TEST_CASE("Ground is only where a solid tile's collider is", "[NavigationSurfaces]")
{
    TileMap tileMap = aMapOf({{{3, Floor}, InsetFromTheLeft}});

    REQUIRE_FALSE(tileMap.groundAt({4, Floor}).has_value());
    REQUIRE_FALSE(tileMap.groundAt({-1, Floor}).has_value());
    std::optional<AABB> ground = tileMap.groundAt({3, Floor});
    REQUIRE(ground);
    REQUIRE(ground->left() == 50.0f);
    REQUIRE(ground->top() == FloorTop);
}

TEST_CASE("A jump that comes down a hair above a collider rests on its top", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 0, 29, APixelLower);
    layRow(laid, 6, 12, 16, APixelLower);
    TileMap tileMap = aMapOf(laid, 30, 12);
    NavigationProfile profile = jumperProfile();

    JumpAttempt attempt = simulateJumpAgainst(
        tileMap,
        profile.abilities,
        profile.physicsBodyData,
        glm::vec2(135.07f, FloorTop + 1.0f),
        1.0f,
        0.5f);

    REQUIRE(attempt.landed);
    REQUIRE(attempt.path.back().y == FloorTop + 1.0f);
}

TEST_CASE("A landing is governed by a node of its own height", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 1, 3, Full);
    layRow(laid, Floor, 4, 6, AThinFloorLowDown);
    TileMap tileMap = aMapOf(laid);
    glm::vec2 upperEnd(64.0f, FloorTop);
    glm::vec2 lowerEnd(64.0f, FloorTop + 12.0f);

    for (bool upperFirst : {true, false})
    {
        NavigationGraph graph;
        graph.addNode(0, upperFirst ? upperEnd : lowerEnd);
        graph.addNode(1, upperFirst ? lowerEnd : upperEnd);

        std::optional<int> governing =
            navigation::nodeGoverning(graph, tileMap, glm::vec2(64.2f, FloorTop + 12.0f), 1, 3.0f);

        INFO("upper node added first: " << upperFirst);
        REQUIRE(governing);
        REQUIRE(graph.getNode(*governing).feet == lowerEnd);
    }
}

TEST_CASE("A jump across a gap reaches a platform a step lower", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 0, 5, Full);
    layRow(laid, Floor, 9, 14, APixelLower);
    TileMap tileMap = aMapOf(laid, 16, 12);

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    bool jumpsAcross = false;
    for (const NavigationEdge &edge : graph.getEdges())
        jumpsAcross = jumpsAcross || (edge.type == EdgeType::Jump &&
                                      graph.getNode(edge.fromId).feet.y == FloorTop &&
                                      graph.getNode(edge.toId).feet.y == FloorTop + 1.0f);
    REQUIRE(jumpsAcross);
}
