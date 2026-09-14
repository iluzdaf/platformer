#include <catch2/catch_test_macros.hpp>
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/jump_ability_data.hpp"
#include "timing/fixed_time_step.hpp"
#include "navigation/input_program.hpp"
#include "navigation/footing.hpp"
#include <catch2/catch_approx.hpp>
#include <cmath>
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
    constexpr int APostAtTheRight = 6;
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
             {AThinFloorLowDown, solidWith(TileColliderData{{0.0f, 12.0f}, {16.0f, 4.0f}})},
             {APostAtTheRight, solidWith(TileColliderData{{12.0f, 0.0f}, {4.0f, 16.0f}})}});
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

TEST_CASE("A fall lands beside a corner no higher than the body steps", "[NavigationSurfaces]")
{
    Placed laid;
    layColumn(laid, 5, 2, Floor - 1, InsetFromTheLeft);
    for (int column = 6; column <= 9; ++column)
        layColumn(laid, column, 2, Floor - 1, Full);
    layRow(laid, Floor, 0, 4, APixelLower);
    layRow(laid, Floor, 5, 9, Full);

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid), jumperProfile());

    std::optional<int> ledge = graph.nodeAtPosition({82.0f, 32.0f});
    REQUIRE(ledge);
    bool fallsDown = false;
    for (const NavigationEdge &leaving : graph.getOutgoingEdges(*ledge))
        fallsDown = fallsDown || (leaving.type == EdgeType::Fall &&
                                  graph.getNode(leaving.toId).feet.y == FloorTop + 1.0f);
    REQUIRE(fallsDown);
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
    layRow(laid, Floor, 0, 29, Full);
    layRow(laid, 5, 0, 4, Full);
    TileMap tileMap = aMapOf(laid, 30, 12);
    NavigationProfile profile = jumperProfile();

    JumpAttempt attempt = simulateJumpAgainst(
        tileMap, profile.abilities, profile.physicsBodyData, glm::vec2(80.0f, 80.0f), 1.0f, 0.25f);

    REQUIRE(attempt.landed);
    REQUIRE(attempt.path.back().y == FloorTop);
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

TEST_CASE("A jump comes down with its feet over the collider it lands on", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, 5, 0, 2, Full);
    laid.push_back({{3, 5}, InsetFromTheRight});
    layRow(laid, Floor - 2, 5, 15, Full);

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid, 16, 12), jumperProfile());

    bool jumpsUp = false;
    for (const NavigationEdge &edge : graph.getEdges())
        if (edge.type == EdgeType::Jump && edge.path.back().y == 80.0f)
        {
            jumpsUp = true;
            INFO("comes down at " << edge.path.back().x << ", the collider ending at 62");
            REQUIRE(edge.path.back().x <= 62.0f);
        }
    REQUIRE(jumpsUp);
}

TEST_CASE(
    "A ledge whose collider stops short of its tile is still jumped onto",
    "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, 4, 0, 1, Full);
    laid.push_back({{2, 4}, InsetFromTheRight});
    layRow(laid, Floor - 2, 5, 15, Full);

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid, 16, 12), jumperProfile());

    bool jumpsUp = false;
    for (const NavigationEdge &edge : graph.getEdges())
        jumpsUp = jumpsUp || (edge.type == EdgeType::Jump && edge.path.back().y == 64.0f &&
                              edge.path.back().x <= 46.0f);
    REQUIRE(jumpsUp);
}

TEST_CASE("Feet are over ground to within a settle of the collider's edges", "[NavigationSurfaces]")
{
    TileMap tileMap = aMapOf({{{2, Floor}, Full}, {{5, Floor}, InsetFromTheRight}});

    REQUIRE(navigation::feetOverGround(tileMap, {32.0f, FloorTop}));
    REQUIRE(navigation::feetOverGround(tileMap, {48.0f, FloorTop}));
    REQUIRE(navigation::feetOverGround(tileMap, {79.5f, FloorTop}));
    REQUIRE(navigation::feetOverGround(tileMap, {94.5f, FloorTop}));
    REQUIRE_FALSE(navigation::feetOverGround(tileMap, {79.0f, FloorTop}));
    REQUIRE_FALSE(navigation::feetOverGround(tileMap, {95.0f, FloorTop}));
    REQUIRE_FALSE(navigation::feetOverGround(tileMap, {64.0f, FloorTop}));
}

namespace
{
    Placed aNarrowLedgeWithAnotherBeyond(int narrow, int gap, int row, bool onTheLeft)
    {
        auto column = [&](int x) { return onTheLeft ? x : 15 - x; };
        Placed laid;
        laid.push_back({{column(narrow), row}, Full});
        for (int x = 0; x <= narrow - gap; ++x)
            laid.push_back({{column(x), row}, Full});
        for (int x = narrow + 1; x <= 15; ++x)
            laid.push_back({{column(x), Floor - 2}, Full});
        return laid;
    }

    Placed aLedgeInsetFromTheLeft()
    {
        Placed laid;
        layRow(laid, 4, 0, 1, Full);
        laid.push_back({{2, 4}, InsetFromTheRight});
        layRow(laid, Floor - 2, 2, 15, Full);
        return laid;
    }
}

TEST_CASE(
    "A jump is kept only if it lands on its run from anywhere a walker takes off for it",
    "[NavigationSurfaces]")
{
    struct Scene
    {
        Placed laid;
        float width;
    };
    std::vector<Scene> scenes{
        {aLedgeInsetFromTheLeft(), 11.0f},
        {aNarrowLedgeWithAnotherBeyond(5, 2, 4, true), 5.0f},
        {aNarrowLedgeWithAnotherBeyond(5, 2, 6, true), 5.0f},
        {aNarrowLedgeWithAnotherBeyond(5, 2, 5, false), 5.0f}};

    for (const Scene &scene : scenes)
    {
        TileMap tileMap = aMapOf(scene.laid, 16, 12);
        NavigationProfile profile = jumperProfile();
        profile.physicsBodyData.colliderSize.x = scene.width;
        NavigationGraph graph = buildNavigationGraph(tileMap, profile);

        for (const NavigationEdge &edge : graph.getEdges())
        {
            if (edge.type != EdgeType::Jump)
                continue;

            for (float offset : {-TakeOffReach, TakeOffReach})
            {
                JumpAttempt again = simulateInputsAgainst(
                    tileMap,
                    profile.abilities,
                    profile.physicsBodyData,
                    edge.path.front() + glm::vec2(offset, 0.0f),
                    edge.inputs,
                    edge.path.back().x);
                INFO(
                    "jumping from " << edge.path.front().x << "," << edge.path.front().y << " to "
                                    << edge.path.back().x << "," << edge.path.back().y
                                    << ", taking off " << offset << " from it");
                REQUIRE(again.landed);
                REQUIRE(again.path.back().y == edge.path.back().y);
                REQUIRE(std::abs(again.path.back().x - edge.path.back().x) <= 2.0f * TakeOffReach);
            }
        }
    }
}

TEST_CASE("A jump that lands on the very corner of its run is kept", "[NavigationSurfaces]")
{
    struct Corner
    {
        Placed laid;
        float width;
        glm::vec2 at;
    };
    Placed onTheLeft;
    layRow(onTheLeft, 4, 0, 2, Full);
    layRow(onTheLeft, Floor - 2, 3, 15, Full);
    Placed onTheRight;
    layRow(onTheRight, 4, 13, 15, Full);
    layRow(onTheRight, Floor - 2, 0, 12, Full);
    Placed insetOnTheRight;
    layRow(insetOnTheRight, 5, 14, 15, Full);
    insetOnTheRight.push_back({{13, 5}, InsetFromTheLeft});
    layRow(insetOnTheRight, Floor - 2, 0, 10, Full);
    std::vector<Corner> corners{
        {onTheLeft, 8.0f, {48.0f, 64.0f}},
        {onTheRight, 8.0f, {208.0f, 64.0f}},
        {insetOnTheRight, 5.0f, {210.0f, 80.0f}}};

    for (const Corner &corner : corners)
    {
        NavigationProfile profile = jumperProfile();
        profile.physicsBodyData.colliderSize.x = corner.width;
        NavigationGraph graph = buildNavigationGraph(aMapOf(corner.laid, 16, 12), profile);

        bool ontoTheCorner = false;
        for (const NavigationEdge &edge : graph.getEdges())
            ontoTheCorner = ontoTheCorner ||
                            (edge.type == EdgeType::Jump && edge.path.back().y == corner.at.y &&
                             std::abs(edge.path.back().x - corner.at.x) <= 2.0f);
        INFO("the corner at " << corner.at.x << "," << corner.at.y);
        REQUIRE(ontoTheCorner);
    }
}

TEST_CASE(
    "A jump from the very end of its run is not held to where the walker cannot stand",
    "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, 5, 0, 1, Full);
    laid.push_back({{2, 5}, InsetFromTheRight});
    layRow(laid, Floor - 2, 4, 15, Full);
    TileMap tileMap = aMapOf(laid, 16, 12);
    NavigationProfile profile = jumperProfile();
    profile.physicsBodyData.colliderSize.x = 5.0f;

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    bool jumpsUp = false;
    for (const NavigationEdge &edge : graph.getEdges())
        jumpsUp = jumpsUp || (edge.type == EdgeType::Jump && edge.path.front().x == 64.0f &&
                              edge.path.back().y == 80.0f);
    REQUIRE(jumpsUp);
}

TEST_CASE("A jump that lands while still held records only what it pressed", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 0, 9, Full);
    layRow(laid, Floor - 2, 0, 9, Full);
    TileMap tileMap = aMapOf(laid);
    NavigationProfile profile = jumperProfile();

    JumpAttempt attempt = simulateJumpAgainst(
        tileMap,
        profile.abilities,
        profile.physicsBodyData,
        glm::vec2(64.0f, FloorTop),
        1.0f,
        1.0f);

    REQUIRE(attempt.landed);
    REQUIRE(durationOf(attempt.inputs) < profile.abilities.jump->jumpDuration);
    REQUIRE(durationOf(attempt.inputs) == Catch::Approx(attempt.steps * PhysicsStep));
}

TEST_CASE("A fall into a wall is refused", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 0, 9, Full);
    layColumn(laid, 6, 0, Floor - 1, Full);
    TileMap tileMap = aMapOf(laid);
    NavigationProfile profile = jumperProfile();

    JumpAttempt fall = simulateFallAgainst(
        tileMap, profile.abilities, profile.physicsBodyData, glm::vec2(92.0f, FloorTop), 1.0f);

    REQUIRE_FALSE(fall.landed);
}

TEST_CASE("A step down no deeper than the body steps is walked, not fallen", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, Floor, 1, 3, Full);
    layRow(laid, Floor, 4, 6, APixelLower);

    NavigationGraph graph = buildNavigationGraph(aMapOf(laid), jumperProfile());

    for (const NavigationEdge &edge : graph.getEdges())
        REQUIRE(edge.type != EdgeType::Fall);
}

TEST_CASE(
    "A fall down the face of a wall rests on the floor under its feet, not the corner beside it",
    "[NavigationSurfaces]")
{
    Placed laid;
    layColumn(laid, 5, 2, Floor - 1, InsetFromTheLeft);
    for (int column = 6; column <= 9; ++column)
        layColumn(laid, column, 2, Floor - 1, Full);
    layRow(laid, Floor, 0, 4, APixelLower);
    layRow(laid, Floor, 5, 9, Full);
    TileMap tileMap = aMapOf(laid);

    for (float width : {4.0f, 5.0f, 6.0f})
    {
        NavigationProfile profile = jumperProfile();
        profile.physicsBodyData.colliderSize.x = width;
        profile.abilities.move->moveSpeed = 60.0f;

        JumpAttempt fall = simulateFallAgainst(
            tileMap, profile.abilities, profile.physicsBodyData, glm::vec2(82.0f, 32.0f), -1.0f);

        INFO("a body " << width << " wide");
        REQUIRE(fall.landed);
        REQUIRE(fall.path.back().y == FloorTop + 1.0f);
    }
}

TEST_CASE("A fall onto ground the graph cannot stand on is no edge", "[NavigationSurfaces]")
{
    Placed laid;
    layRow(laid, 5, 0, 3, Full);
    layColumn(laid, 4, 0, Floor - 1, APostAtTheRight);
    layRow(laid, Floor, 0, 9, Full);
    TileMap tileMap = aMapOf(laid);
    NavigationProfile profile = jumperProfile();

    JumpAttempt fall = simulateFallAgainst(
        tileMap, profile.abilities, profile.physicsBodyData, glm::vec2(64.0f, 80.0f), 1.0f);
    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(fall.landed);
    REQUIRE(tileMap.tileContaining(fall.path.back()).x == 4);
    for (const NavigationEdge &edge : graph.getEdges())
        REQUIRE(edge.type != EdgeType::Fall);
}

TEST_CASE(
    "A fall is kept only if it lands on its run from anywhere a walker takes off for it",
    "[NavigationSurfaces]")
{
    Placed withoutALedge;
    layRow(withoutALedge, 3, 0, 3, Full);
    layRow(withoutALedge, Floor + 1, 0, 11, Full);
    Placed withALedgeBelow = withoutALedge;
    withALedgeBelow.push_back({{4, 5}, InsetFromTheLeft});
    NavigationProfile profile = jumperProfile();
    profile.physicsBodyData.colliderSize.x = 5.0f;
    profile.abilities.move->moveSpeed = 400.0f;

    int fallsChecked = 0;
    for (const Placed &laid : {withoutALedge, withALedgeBelow})
    {
        TileMap tileMap = aMapOf(laid, 12, 12);
        NavigationGraph graph = buildNavigationGraph(tileMap, profile);

        for (const NavigationEdge &edge : graph.getEdges())
        {
            if (edge.type != EdgeType::Fall)
                continue;

            ++fallsChecked;
            for (float offset : {-TakeOffReach, TakeOffReach})
            {
                JumpAttempt again = simulateInputsAgainst(
                    tileMap,
                    profile.abilities,
                    profile.physicsBodyData,
                    edge.path.front() + glm::vec2(offset, 0.0f),
                    edge.inputs,
                    edge.path.back().x);
                INFO(
                    "falling from " << edge.path.front().x << "," << edge.path.front().y
                                    << ", taking off " << offset << " from it");
                if (again.steps == 0)
                    continue;

                REQUIRE(again.landed);
                REQUIRE(again.path.back().y == edge.path.back().y);
            }
        }
    }
    REQUIRE(fallsChecked > 0);
}
