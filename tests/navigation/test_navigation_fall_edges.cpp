#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/tiles.hpp"
#include "helpers/navigation_maps.hpp"
#include "helpers/palettes.hpp"
#include "helpers/graph_queries.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_graph_builder.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "timing/fixed_time_step.hpp"
#include "navigation/input_program.hpp"
#include "navigation/jump_simulation.hpp"
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
            REQUIRE(graph.getNode(edge.toId).feet.y > graph.getNode(edge.fromId).feet.y);
}

TEST_CASE("Falling is not offered where you could walk", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
}

TEST_CASE("A profile that cannot move has no fall to take", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    NavigationProfile profile = profileThatMoves(13.0f, fallerAbilities());

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) == 0);
    REQUIRE(countEdgesOfType(graph, EdgeType::Jump) == 0);
}

TEST_CASE("A slow actor can still step off a ledge", "[NavigationGraphBuilder][Fall]")
{
    AbilitiesData slow = jumperAbilities();
    slow.move->moveSpeed = 60.0f;

    NavigationProfile profile = profileThatMoves(13.0f, slow);
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, profile);

    REQUIRE(countEdgesOfType(graph, EdgeType::Fall) > 0);
}

TEST_CASE("A fall is drawn as the path it takes", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();

    NavigationGraph graph = buildNavigationGraph(tileMap, jumperProfile());

    int falls = 0;
    for (const auto &edge : graph.getEdges())
        if (edge.type == EdgeType::Fall)
        {
            ++falls;
            REQUIRE(edge.path.size() > 2);
            REQUIRE(edge.path.front() == graph.getNode(edge.fromId).feet);
            REQUIRE(edge.path.back().y == graph.getNode(edge.toId).feet.y);
        }
    REQUIRE(falls > 0);
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
    layRow(laid, PlatformRow + 1, 0, 19, SpikeTile);
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
        float stepOff = std::abs(to.feet.x - from.feet.x);
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

    glm::vec2 comesDown(takeOffPosition(tileMap).x + 8.0f, static_cast<float>(FloorBelowRow * 16));
    std::vector<navigation::ChosenFall> falls{{0, {takeOffPosition(tileMap), comesDown}, {}}};

    navigation::addFallEdges(graph, tileMap, jumperProfile(), 1, falls);

    REQUIRE(graph.getEdges().empty());
}

TEST_CASE("A fall walks off its ledge and then lets go", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    NavigationProfile profile = jumperProfile();

    JumpAttempt fall = simulateFallAgainst(
        tileMap, profile.abilities, profile.physicsBodyData, takeOffPosition(tileMap), 1.0f);

    REQUIRE(fall.landed);
    REQUIRE(fall.path.back().y == static_cast<float>(FloorBelowRow * 16));
    REQUIRE(fall.inputs.size() == 1);
    REQUIRE(fall.inputs.front().pressed.direction.x == 1.0f);
    REQUIRE(fall.inputs.front().duration > 0.0f);
    float pushedFor = fall.inputs.front().duration * profile.abilities.move->moveSpeed;
    REQUIRE(
        pushedFor <= profile.physicsBodyData.colliderSize.x * 0.5f +
                         profile.abilities.move->moveSpeed * PhysicsStep);
}

TEST_CASE("A fall asked of a node away from its ledge is refused", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    NavigationProfile profile = jumperProfile();
    glm::vec2 wellBack = takeOffPosition(tileMap) - glm::vec2(32.0f, 0.0f);

    JumpAttempt fall =
        simulateFallAgainst(tileMap, profile.abilities, profile.physicsBodyData, wellBack, 1.0f);

    REQUIRE_FALSE(fall.landed);
}

TEST_CASE("A fall leaves from the end of its run", "[NavigationGraphBuilder][Fall]")
{
    TileMap tileMap = aLedgeAboveAFloor();
    glm::vec2 end = takeOffPosition(tileMap);
    NavigationGraph graph;
    graph.addNode(0, {0.0f, end.y});
    graph.addNode(1, end);
    graph.addNode(2, end - glm::vec2(3.0f, 0.0f));

    std::vector<navigation::ChosenFall> falls =
        navigation::addFallLandingNodes(graph, tileMap, jumperProfile(), 1);

    REQUIRE_FALSE(falls.empty());
    for (const navigation::ChosenFall &fall : falls)
        REQUIRE(fall.fromId == 1);
}
